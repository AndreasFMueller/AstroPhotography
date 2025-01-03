/*
 * guidercontrollerwidget.cpp -- implementation of the guider controller
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guidercontrollerwidget.h"
#include "ui_guidercontrollerwidget.h"
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <AstroCamera.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <algorithm>
#include "trackselectiondialog.h"
#include "trackviewdialog.h"
#include "trackingmonitordialog.h"

using namespace astro::camera;
using namespace astro::image;

namespace snowgui {

/**
 * \brief Auxiliary class to convert dial positions to filter values and back
 *
 * The dial indicators to modify the parameters have the same range of integer
 * values, but the filters themselves should be using different ranges
 * of the parameters depending of the 
 */
class GuiderParameterConverter {
	snowstar::FilterMethod  _method;
public:
	GuiderParameterConverter(snowstar::FilterMethod method)
		: _method(method) { }
	int     parameter2dial(float value) const;
	float  dial2parameter(int dial) const;
	int	operator()(float v) const { return parameter2dial(v); }
	float	operator()(int d) const { return dial2parameter(d); }
};

/**
 * \brief Convert a dial position into a filter parameter value
 *
 * \param dial	dial position to convert
 */
float	GuiderParameterConverter::dial2parameter(int dial) const {
	switch (_method) {
	case snowstar::FilterNONE:
		return 1.0;
	case snowstar::FilterGAIN:
		return 0.2 + 0.01 * dial;
	case snowstar::FilterKALMAN:
		return 0.2 + dial / 8.;
	}
	std::string	msg = astro::stringprintf("unknown method %d", _method);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Convert a filter parameter value into a dial position
 *
 * \param value		value to convert to a dial position
 */
int	GuiderParameterConverter::parameter2dial(float value) const {
	switch (_method) {
	case snowstar::FilterNONE:
		return 80;
	case snowstar::FilterGAIN:
		{
			int	dial = 100 * (value - 0.2);
			if (dial < 0) { dial = 0; }
			if (dial > 160) { dial = 160; }
			return dial;
		}
	case snowstar::FilterKALMAN:
		{
			int	dial = 8 * (value - 0.2);
			if (dial < 0) { dial = 0; }
			if (dial > 160) { dial = 160; }
			return dial;
		}
	}
	std::string	msg = astro::stringprintf("unknown method %d", _method);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Constructor for the guidercontrollerwidget
 */
guidercontrollerwidget::guidercontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::guidercontrollerwidget) {
	ui->setupUi(this);

	// adding items to the tracking method combo box
	ui->trackingMethodBox->addItem(QString("Star"));
	ui->trackingMethodBox->addItem(QString("Phase"));
	ui->trackingMethodBox->addItem(QString("Gradient"));
	ui->trackingMethodBox->addItem(QString("Laplace"));
	ui->trackingMethodBox->addItem(QString("Large"));
	connect(ui->trackingMethodBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(trackingMethodChanged(int)));

	// add the items to the filter method combo box
	ui->filterMethodBox->addItem(QString("None"));
	ui->filterMethodBox->addItem(QString("Gain"));
	ui->filterMethodBox->addItem(QString("Kalman"));
	ui->filterMethodBox->setCurrentIndex(1);
	connect(ui->filterMethodBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(filterMethodChanged(int)));

	// connections for other GUI elements
	connect(ui->gpupdateintervalSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(gpupdateintervalChanged(double)));
	connect(ui->aoupdateintervalSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(aoupdateintervalChanged(double)));
	connect(ui->windowradiusSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(windowradiusChanged(int)));
	_windowradius = 50;

	connect(ui->gpFlipBox, SIGNAL(stateChanged(int)),
		this, SLOT(gpFlipStateChanged(int)));
	connect(ui->aoFlipBox, SIGNAL(stateChanged(int)),
		this, SLOT(aoFlipStateChanged(int)));

	connect(ui->guideButton, SIGNAL(clicked()),
		this, SLOT(startGuiding()));
	connect(ui->moreButton, SIGNAL(clicked()),
		this, SLOT(showMoreMenu()));

	connect(ui->xGainDial, SIGNAL(valueChanged(int)),
		this, SLOT(xGainChanged(int)));
	connect(ui->yGainDial, SIGNAL(valueChanged(int)),
		this, SLOT(yGainChanged(int)));

	// connections for the calculator
	connect(this, SIGNAL(telescopeChanged(astro::RaDec)),
		ui->gpcalibrationWidget, SLOT(setTelescope(astro::RaDec)));
	connect(this, SIGNAL(telescopeChanged(astro::RaDec)),
		ui->aocalibrationWidget, SLOT(setTelescope(astro::RaDec)));

	connect(this, SIGNAL(orientationChanged(bool)),
		ui->gpcalibrationWidget, SLOT(setOrientation(bool)));
	connect(this, SIGNAL(orientationChanged(bool)),
		ui->aocalibrationWidget, SLOT(setOrientation(bool)));

	connect(ui->gpcalibrationWidget, SIGNAL(calibrationChanged()),
		this, SLOT(gpCalibrationChanged()));
	connect(ui->aocalibrationWidget, SIGNAL(calibrationChanged()),
		this, SLOT(aoCalibrationChanged()));

	// create the tracking monitor
	_trackingmonitordialog = NULL;
	_trackingmonitor = NULL;
	_trackinglabel = new QLabel(NULL);
	ui->trackingImageArea->setWidget(_trackinglabel);
	_trackingmonitorimage = new TrackingMonitorImage(this, _trackinglabel);
	_trackingmonitorimageptr = Ice::ObjectPtr(_trackingmonitorimage);

	connect(ui->freezeButton, SIGNAL(toggled(bool)),
		this, SLOT(toggleFreeze(bool)));
	connect(ui->inverseBox, SIGNAL(toggled(bool)),
		this, SLOT(toggleInverse(bool)));
	connect(ui->imageStepSpinBox, SIGNAL(valueChanged(int)),
		_trackingmonitorimage, SLOT(setScale(int)));
	connect(ui->monitorButton, SIGNAL(clicked()),
		this, SLOT(launchMonitor()));

	connect(_trackingmonitorimage, SIGNAL(imageUpdated()),
		this, SLOT(imageUpdated()));

	// set the font for the time
	QFont	f("Microsoft Sans Serif");;
	f.setStyleHint(QFont::Monospace);
	ui->timeLabel->setFont(f);

	// some other fields
	_backlashDialog = NULL;
	_previousstate = snowstar::GuiderIDLE;
	statusTimer.setInterval(100);
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	_gpupdateinterval = 3;
	_aoupdateinterval = 1;
	_stepping = false;
}

/**
 * \brief Instrument setup for the guidercontrollerwidget
 *
 * This method also creates the guider factory and the guidre descriptor.
 * The guider itself is set up in setGuider() method
 */
void	guidercontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// create the guiderfactory proxy
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
	astro::ServerName	servername(serviceobject.name());
	Ice::ObjectPrx  gbase
		= ic->stringToProxy(servername.connect("Guiders"));
	try {
		_guiderfactory = snowstar::GuiderFactoryPrx::checkedCast(gbase);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get a guider factory: %s",
			x.what());
	}

	// now build a GuiderDescriptor for the guider
	_instrumentname = _instrument.name();
}

/**
 * \brief Main thread initializations
 */
void	guidercontrollerwidget::setupComplete() {
	// set up the guider
	setupGuider();
}

/**
 * \brief Setup a guider
 */
void	guidercontrollerwidget::setupGuider() {
	std::string	guidername = _instrumentname;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up the guider %s",
		guidername.c_str());
	statusTimer.stop();

	// we cannot do anything if we don't have a guiderfactory
	if (!_guiderfactory) {
		QMessageBox	message;
		message.setText(QString("No guider factory"));
		std::ostringstream	out;
		out << "A connection to the guider factory to retrieve the ";
		out << "guider " << guidername << "failed. The guider could";
		out << "not be set upt." << std::endl;
		message.setInformativeText(QString(out.str().c_str()));
		message.exec();
		return;
	}

	// get the guider based on the descriptor
	try {
		_guider = _guiderfactory->get(_instrumentname);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got the guider %s",
			guidername.c_str());
	} catch (const std::exception& x) {
		QMessageBox	message;
		std::string	title
			= astro::stringprintf("Cannot connect to %s",
			guidername.c_str());
		message.setText(QString(title.c_str()));
		std::ostringstream	out;
		out << "The connection to ";
		out << guidername;
		out << " was not possible: ";
		out << x.what();
		out << std::endl;
		message.setInformativeText(QString(out.str().c_str()));
		message.exec();
	}

	// also propagate the information to the calibration widgets
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting guider '%s' in calwidgets",
		_instrumentname.c_str());
	ui->gpcalibrationWidget->setGuider(snowstar::ControlGuidePort,
		_instrumentname, _guider, _guiderfactory, this);
	ui->aocalibrationWidget->setGuider(snowstar::ControlAdaptiveOptics,
		_instrumentname, _guider, _guiderfactory, this);

	// we cannot do much more without a guider
	if (!_guider) {
		return;
	}

	// get the calibration star
	try {
		snowstar::Point	star = _guider->getStar();
		int	x = star.x;
		int	y = star.y;
		if ((x >= 0) && (y >= 0)) {
			ui->starxField->setText(QString(
				astro::stringprintf("%d", x).c_str()));
			ui->staryField->setText(QString(
				astro::stringprintf("%d", y).c_str()));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting guide star to (%d,%d)",
			x, y);
	} catch (snowstar::BadState& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad state: %s", x.what());
	}

	// get the information from the guider
	ui->trackingMethodBox->blockSignals(true);
	try {
		switch (_guider->getTrackerMethod()) {
		case snowstar::TrackerUNDEFINED:
		case snowstar::TrackerNULL:
		case snowstar::TrackerSTAR:
			ui->trackingMethodBox->setCurrentIndex(0);
			break;
		case snowstar::TrackerPHASE:
			ui->trackingMethodBox->setCurrentIndex(1);
			break;
		case snowstar::TrackerDIFFPHASE:
			ui->trackingMethodBox->setCurrentIndex(2);
			break;
		case snowstar::TrackerLAPLACE:
			ui->trackingMethodBox->setCurrentIndex(3);
			break;
		case snowstar::TrackerLARGE:
			ui->trackingMethodBox->setCurrentIndex(4);
			break;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking method set");
	} catch (snowstar::BadState& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot get tracking method: bad state: %s", x.what());
	}
	ui->trackingMethodBox->blockSignals(false);

	// get some information about the configuration
	try {
		snowstar::Calibration	gpcalibration
			= _guider->getCalibration(snowstar::ControlGuidePort);
		ui->gpFlipBox->setCheckState((gpcalibration.meridianFlipped)
			? Qt::Checked : Qt::Unchecked);
		ui->gpFlipBox->setEnabled(true);
		ui->gpupdateintervalSpinBox->setEnabled(true);
	} catch (...) {
		ui->gpFlipBox->setEnabled(false);
		ui->gpupdateintervalSpinBox->setEnabled(false);
	}
	try {
		snowstar::Calibration	gpcalibration
			= _guider->getCalibration(snowstar::ControlAdaptiveOptics);
		ui->aoFlipBox->setCheckState((gpcalibration.meridianFlipped)
			? Qt::Checked : Qt::Unchecked);
		ui->aoFlipBox->setEnabled(true);
		ui->aoupdateintervalSpinBox->setEnabled(true);
	} catch (...) {
		ui->aoFlipBox->setEnabled(false);
		ui->aoupdateintervalSpinBox->setEnabled(false);
	}

	// get the exposure information
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting exposure info");
		_exposure = snowstar::convert(_guider->getExposure());
		astro::Point	ps = snowstar::convert(_guider->getStar());
		_star = ImagePoint((int)ps.x(), (int)ps.y());

		ui->starxField->setText(QString::number(_star.x()));
		ui->staryField->setText(QString::number(_star.y()));

		ui->windowradiusSpinBox->blockSignals(true);
		_windowradius = std::min(_exposure.frame().size().width(),
			_exposure.frame().size().height()) / 2;
		ui->windowradiusSpinBox->setValue(_windowradius);
		ui->windowradiusSpinBox->blockSignals(false);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "window radius set to %d",
			_windowradius);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"cannot get exposure information: %s", x.what());
	}

	// get the filter method
	ui->filterMethodBox->blockSignals(true);
	try {
		snowstar::FilterMethod	filtermethod
			= _guider->getFilterMethod();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "current filter method: %d",
			filtermethod);
		switch (filtermethod) {
		case snowstar::FilterNONE:
			ui->filterMethodBox->setCurrentIndex(0);
			break;
		case snowstar::FilterGAIN:
			ui->filterMethodBox->setCurrentIndex(1);
			break;
		case snowstar::FilterKALMAN:
			ui->filterMethodBox->setCurrentIndex(2);
			break;
		}
		setupFilter();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get filter method: %s",
			x.what());
	}
	ui->filterMethodBox->blockSignals(false);

	// get the guiding interval
	try {
		float	interval = _guider->getGuidingInterval();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got guide interval %.3f",
			interval);
		ui->gpupdateintervalSpinBox->blockSignals(true);
		ui->gpupdateintervalSpinBox->setValue(interval);
		ui->gpupdateintervalSpinBox->blockSignals(false);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get the guide interval");
	}

	// gain
	try {
		GuiderParameterConverter	gpc(_guider->getFilterMethod());
		int gx = gpc.parameter2dial(_guider->getFilterParameter(0));
		ui->xGainDial->setValue(gx);
		int gy = gpc.parameter2dial(_guider->getFilterParameter(1));
		ui->yGainDial->setValue(gy);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot set filter gains: %s",
			x.what());
	}
	
	// do all the registration stuff
	_trackingmonitorimage->setGuider(_guider, _trackingmonitorimageptr);

	// start the timer
	statusTimer.start();
}

/**
 * \brief Destructor for the guidercontrollerwidget
 *
 * The destructor also stops and destroys the timer
 */
guidercontrollerwidget::~guidercontrollerwidget() {
	statusTimer.stop();
	if (_trackingmonitor) {
		Ice::Identity	identity = _trackingmonitor->identity();
		snowstar::CommunicatorSingleton::remove(identity);
	}
	delete ui;
}

/**
 * \brief Set the exposure to use for the guider
 */
void	guidercontrollerwidget::setExposure(astro::camera::Exposure exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting the exposure: %s",
		exposure.toString().c_str());
	try {
		switch (_guider->getState()) {
		case snowstar::GuiderUNCONFIGURED:
		case snowstar::GuiderIDLE:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "setting exposure state is ok");
			break;
		default:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "setting exposure not allowed in this state");
			QMessageBox	message(this);
			message.setText(QString("cannot set exposure"));
			message.setInformativeText(QString("The exposure can only be changed when the imager is currently not in use"));
			message.setStandardButtons(QMessageBox::Ok);
			message.exec();
			return;
		}
		_exposure = exposure;
		_guider->setExposure(snowstar::convert(_exposure));
		// XXX star was already set
		// use the center point as the star
		//setStar(_exposure.frame().size().center());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not set exposure");
	}
}

/**
 * \brief Slot to change the star
 */
void	guidercontrollerwidget::setStar(astro::image::ImagePoint star) {
	_star = star;
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting star (%d,%d)",
			star.x(), star.y());
		snowstar::Point	p;
		p.x = star.x();
		p.y = star.y();
		_guider->setStar(p);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set star");
	}
}

/**
 * \brief Select the point around which guiding operations will take place
 *
 * The precise point si only used by the star tracker, but the other methods
 * also need a subwindow defined by the 
 */
void	guidercontrollerwidget::selectPoint(astro::image::ImagePoint p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "point %s selected",
		p.toString().c_str());
	// use the current frame from the exposure structure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "_exposure.frame() = %s",
		_exposure.frame().toString().c_str());
	astro::image::ImagePoint	ip = p;
	setStar(ip);
	ui->starxField->setText(QString::number(ip.x()));
	ui->staryField->setText(QString::number(ip.y()));
}

/**
 * \brief Set up the tracker
 */
void	guidercontrollerwidget::setupTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tracker setup: window radius: %d",
		_windowradius);
	// get the current exposure settings
	Exposure	exposure = snowstar::convert(_guider->getExposure());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tracker setup: current exposure: %s",
		exposure.toString().c_str());

	// compute the window for exposures
	ImagePoint	origin(_star.x() - _windowradius,
				_star.y() - _windowradius);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tracker setup: origin: %s",
		origin.toString().c_str());
	ImageSize	size(2 * _windowradius, 2 * _windowradius);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tracker setup: size: %s",
		size.toString().c_str());
	exposure.frame(ImageRectangle(origin, size));
	_guider->setExposure(snowstar::convert(exposure));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Tracker setup: exposure set to: %s",
		exposure.toString().c_str());
}

/**
 * \brief Start guiding
 */
void	guidercontrollerwidget::startGuiding() {
	if (!_guider) {
		return;
	}
	// first handle the simple case that it is already guiding: stop it
	try {
		if (snowstar::GuiderGUIDING == _guider->getState()) {
			_guider->stopGuiding();
			return;
		}
	} catch (...) {
	}

	// make sure that the calibration is correctly flipped if necessary
	checkGPFlipped();

	// prepare the tracker
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding");
		setupTracker();
		_guider->startGuiding(_gpupdateinterval, _aoupdateinterval,
			_stepping);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding started");
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start guiding");
	}
}

/**
 * \brief Stop guiding
 */
void	guidercontrollerwidget::stopGuiding() {
	if (!_guider) {
		return;
	}
	try {
		_guider->stopGuiding();
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop guiding");
	}
}

/**
 * \brief Slot to update the status display
 */
void	guidercontrollerwidget::statusUpdate() {
	if (!_guider) {
		return;
	}
	ui->timeLabel->setText(QString(astro::Timer::timestamp(1).c_str()));
	snowstar::GuiderState	state = _guider->getState();
	if (state == _previousstate) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state: %d", (int)state);
	switch (state) {
	case snowstar::GuiderIDLE:
		ui->guideButton->setText(QString("Guiding"));
		ui->guideButton->setEnabled(false);
		ui->monitorButton->setEnabled(false);
		ui->gpFlipBox->setEnabled(false);
		ui->aoFlipBox->setEnabled(false);
		ui->gpupdateintervalSpinBox->setEnabled(false);
		ui->aoupdateintervalSpinBox->setEnabled(false);
		break;
	case snowstar::GuiderUNCONFIGURED:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(false);
		ui->monitorButton->setEnabled(false);
		ui->gpFlipBox->setEnabled(false);
		ui->aoFlipBox->setEnabled(false);
		ui->gpupdateintervalSpinBox->setEnabled(false);
		ui->aoupdateintervalSpinBox->setEnabled(false);
		break;
	case snowstar::GuiderCALIBRATING:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(false);
		ui->monitorButton->setEnabled(true);
		ui->gpFlipBox->setEnabled(false);
		ui->aoFlipBox->setEnabled(false);
		ui->gpupdateintervalSpinBox->setEnabled(false);
		ui->aoupdateintervalSpinBox->setEnabled(false);
		break;
	case snowstar::GuiderCALIBRATED:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(true);
		ui->gpFlipBox->setEnabled(true);
		ui->aoFlipBox->setEnabled(true);
		ui->gpupdateintervalSpinBox->setEnabled(true);
		ui->aoupdateintervalSpinBox->setEnabled(true);
		break;
	case snowstar::GuiderGUIDING:
		ui->guideButton->setText(QString("Stop Guiding"));
		ui->guideButton->setEnabled(true);
		ui->monitorButton->setEnabled(true);
		ui->gpFlipBox->setEnabled(true);
		ui->aoFlipBox->setEnabled(true);
		ui->gpupdateintervalSpinBox->setEnabled(true);
		ui->aoupdateintervalSpinBox->setEnabled(true);
		break;
	case snowstar::GuiderBACKLASH:
		ui->gpFlipBox->setEnabled(false);
		ui->aoFlipBox->setEnabled(false);
		[[fallthrough]];
	case snowstar::GuiderIMAGING:
	case snowstar::GuiderDARKACQUIRE:
	case snowstar::GuiderFLATACQUIRE:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(false);
		ui->monitorButton->setEnabled(false);
		ui->gpFlipBox->setEnabled(false);
		ui->aoFlipBox->setEnabled(false);
		ui->gpupdateintervalSpinBox->setEnabled(false);
		ui->aoupdateintervalSpinBox->setEnabled(false);
		break;
	}
	_previousstate = state;
}

/**
 * \brief Change the tracker method
 */
void	guidercontrollerwidget::trackingMethodChanged(int index) {
	switch (index) {
	case 0:	_guider->setTrackerMethod(snowstar::TrackerSTAR);
		break;
	case 1:	_guider->setTrackerMethod(snowstar::TrackerPHASE);
		break;
	case 2:	_guider->setTrackerMethod(snowstar::TrackerDIFFPHASE);
		break;
	case 3:	_guider->setTrackerMethod(snowstar::TrackerLAPLACE);
		break;
	case 4:	_guider->setTrackerMethod(snowstar::TrackerLARGE);
		break;
	}
}

/**
 * \brief modify user interface matching the filter method
 */
void	guidercontrollerwidget::setupFilter() {
	switch (ui->filterMethodBox->currentIndex()) {
	case 0:	
		ui->gainLabel->setText(QString("None"));
		ui->xGainLabel->setText(QString(""));
		ui->yGainLabel->setText(QString(""));
		ui->xGainDial->setEnabled(false);
		ui->yGainDial->setEnabled(false);
		ui->xGainValue->setText(QString(""));
		ui->yGainValue->setText(QString(""));
		break;
	case 1:	
		ui->gainLabel->setText(QString("Gain:"));
		ui->xGainLabel->setText(QString("X:"));
		ui->yGainLabel->setText(QString("Y:"));
		ui->xGainDial->setEnabled(true);
		ui->yGainDial->setEnabled(true);
		break;
	case 2:	
		ui->gainLabel->setText(QString("Errors:"));
		ui->xGainLabel->setText(QString("Sys:"));
		ui->yGainLabel->setText(QString("Meas:"));
		ui->xGainDial->setEnabled(true);
		ui->yGainDial->setEnabled(true);
		break;
	}
}

/**
 * \brief call this method after a filter method change to update the parameters
 */
void	guidercontrollerwidget::updateParameters() {
	xGainChanged(ui->xGainDial->value());
	yGainChanged(ui->yGainDial->value());
}

/**
 * \brief Change the filter method
 */
void	guidercontrollerwidget::filterMethodChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter method changed to %d", index);
	switch (index) {
	case 0:	_guider->setFilterMethod(snowstar::FilterNONE);
		break;
	case 1:	_guider->setFilterMethod(snowstar::FilterGAIN);
		break;
	case 2:	_guider->setFilterMethod(snowstar::FilterKALMAN);
		break;
	}
	setupFilter();
	updateParameters();
}

/**
 * \brief update the guideport change interval
 */
void	guidercontrollerwidget::gpupdateintervalChanged(double r) {
	_gpupdateinterval = r;
}

/**
 * \brief Update the adaptive optics update interval
 *
 * Also makes shure that the minimum value that can be set for the GP
 * update interval is always at least 1 second and also at least the 
 * as large as the update interval for the adaptive optics unit.
 */
void	guidercontrollerwidget::aoupdateintervalChanged(double r) {
	_aoupdateinterval = r;
	double	mingpinterval = std::max(_aoupdateinterval, 1.);
	if (mingpinterval > ui->gpupdateintervalSpinBox->minimum()) {
		ui->gpupdateintervalSpinBox->setMinimum(mingpinterval);
	}
}

/**
 * \brief update the window radius
 */
void	guidercontrollerwidget::windowradiusChanged(int w) {
	_windowradius = w;
}


/**
 * \brief Open a track selection dialog
 */
void	guidercontrollerwidget::selectTrack() {
	trackselectiondialog	*tsd = new trackselectiondialog(this);
	tsd->setGuider(_instrumentname, _guiderfactory);
	tsd->show();
	connect(tsd, SIGNAL(trackSelected(snowstar::TrackingHistory)),
		this, SLOT(trackSelected(snowstar::TrackingHistory)));
}

/**
 * \brief Slot to accept the new track
 */
void	guidercontrollerwidget::trackSelected(snowstar::TrackingHistory track) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open view on track %d",
		track.trackid);
	trackingmonitordialog	*tmd = new trackingmonitordialog(this);
	tmd->add(track);
	if (track.guideportcalid > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve GP cal %d",
			track.guideportcalid);
		snowstar::Calibration cal = _guiderfactory
			->getCalibration(track.guideportcalid);
		tmd->calibration(cal);
	}
	if (track.adaptiveopticscalid > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve AO cal %d",
			track.adaptiveopticscalid);
		snowstar::Calibration cal = _guiderfactory
			->getCalibration(track.adaptiveopticscalid);
		tmd->calibration(cal);
	}
	tmd->show();
	tmd->updateData();
}

/**
 * \brief Toggle 
 */
void	guidercontrollerwidget::toggleFreeze(bool state) {
	if (_trackingmonitorimage) {
		_trackingmonitorimage->setFreeze(state);
	}
}

/**
 * \brief Toggle 
 */
void	guidercontrollerwidget::toggleInverse(bool state) {
	if (_trackingmonitorimage) {
		_trackingmonitorimage->setInverse(state);
	}
}

/**
 * \brief Launch tracking monitor
 */
void	guidercontrollerwidget::launchMonitor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a tracking monitor");
	if (!_guider) {
		return;
	}

	// try to get the tracking history to display
	snowstar::TrackingSummary	summary;
	snowstar::TrackingHistory	history;
	try {
		summary = _guider->getTrackingSummary();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get track %d",
			summary.trackid);
		history = _guider->getTrackingHistory(summary.trackid);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d points",
			history.points.size());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get the history: %s",
			x.what());
		QMessageBox	message(this);
		message.setText(QString("Cannot monitor"));
		message.setInformativeText(QString("Monitoring could not be started as the tracking history could not be retrieved."));
		message.setStandardButtons(QMessageBox::Ok);
		message.exec();
		return;
	}

	// if there is no tracking monitor, try to get one
	if (!_trackingmonitordialog) {
		_trackingmonitordialog = new trackingmonitordialog(this);
	}
	_trackingmonitor = new TrackingMonitorController(NULL,
				_trackingmonitordialog);
	_trackingmonitorptr = Ice::ObjectPtr(_trackingmonitor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracking monitor generated");

	// add the history to the track display
	_trackingmonitordialog->add(history);

	// retrieve the calibrations
	if (history.guideportcalid > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve cal %d",
			history.guideportcalid);
		snowstar::Calibration cal = _guiderfactory
			->getCalibration(history.guideportcalid);
		_trackingmonitordialog->calibration(cal);
	}
	if (history.adaptiveopticscalid > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve AO cal %d",
			history.adaptiveopticscalid);
		snowstar::Calibration cal = _guiderfactory
			->getCalibration(history.adaptiveopticscalid);
		_trackingmonitordialog->calibration(cal);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrations installed");

	// now register the callback, to make sure we get new points only after
	// we have added the history
	_trackingmonitor->setGuider(_guider, _trackingmonitorptr);

	// display the dialog
	_trackingmonitordialog->show();
	_trackingmonitordialog->raise();
}

/**
 * \brief what to do when the image updates
 *
 * This slot updates the timestamp above the most recent tracking image
 */
void	guidercontrollerwidget::imageUpdated() {
	time_t	now;
	time(&now);
	struct tm	*tmp = localtime(&now);
        char	buffer[200];
        strftime(buffer, sizeof(buffer), "%T", tmp);
        std::string	label =  astro::stringprintf("Most recent image: %s",
		buffer);
	ui->trackingLabel->setText(QString(label.c_str()));
}

/**
 * \brief What to do to characterize RA backlash
 */
void	guidercontrollerwidget::backlashRAClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backlash RA clicked");
	setupTracker();
	if (!_backlashDialog) {
		_backlashDialog = new BacklashDialog();
		_backlashDialog->guider(_guider);
		_backlashDialog->show();
	} else {
		_backlashDialog->show();
		_backlashDialog->raise();
		QApplication::setActiveWindow(_backlashDialog);
	}
	_backlashDialog->direction(snowstar::BacklashRA);
	_backlashDialog->setWindowTitle("RA Backlash");
}

/**
 * \brief What to do to characterize DEC backlash
 */
void	guidercontrollerwidget::backlashDECClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backlash DEC clicked");
	setupTracker();
	if (!_backlashDialog) {
		_backlashDialog = new BacklashDialog();
		_backlashDialog->guider(_guider);
		_backlashDialog->show();
	} else {
		_backlashDialog->show();
		_backlashDialog->raise();
		QApplication::setActiveWindow(_backlashDialog);
	}
	_backlashDialog->direction(snowstar::BacklashDEC);
	_backlashDialog->setWindowTitle("DEC Backlash");
}

/**
 * \brief Change the x gain
 *
 * \param value		new value of the Parameter
 */
void	guidercontrollerwidget::xGainChanged(int value) {
	GuiderParameterConverter	gpc(_guider->getFilterMethod());
	float	fvalue = gpc.dial2parameter(value);
	char	b[20];
	snprintf(b, sizeof(b), "%.2f", fvalue);
	ui->xGainValue->setText(QString(b));
	if (_guider) {
		_guider->setFilterParameter(0, fvalue);
	}
}

/**
 * \brief Change the y gain
 *
 * \param value		new value of the Parameter
 */
void	guidercontrollerwidget::yGainChanged(int value) {
	GuiderParameterConverter	gpc(_guider->getFilterMethod());
	float	fvalue = gpc.dial2parameter(value);
	char	b[20];
	snprintf(b, sizeof(b), "%.2f", fvalue);
	ui->yGainValue->setText(QString(b));
	if (_guider) {
		_guider->setFilterParameter(1, fvalue);
	}
}

void	guidercontrollerwidget::setTelescope(astro::RaDec radec) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got new telescope: %s",
		radec.toString().c_str());
	emit telescopeChanged(radec);
}

void	guidercontrollerwidget::setOrientation(bool west) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got new orientation: %s",
		(west) ? "west" : "east");
	emit orientationChanged(west);
}

void	guidercontrollerwidget::gpFlipStateChanged(int /* state */) {
	if (_guider) {
		return;
	}
	gpCalibrationChanged();
}

void	guidercontrollerwidget::aoFlipStateChanged(int /* state */) {
	if (_guider) {
		return;
	}
	aoCalibrationChanged();
}

/**
 * \brief Make sure the GP calibration is properly flipped
 */
void	guidercontrollerwidget::checkGPFlipped() {
	bool	meridianFlipped = (ui->gpFlipBox->checkState() == Qt::Checked);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether GP cal is %sflipped",
		(meridianFlipped) ? "" : "not ");
	try {
		snowstar::Calibration	calibration
			= _guider->getCalibration(snowstar::ControlGuidePort);
		if (calibration.meridianFlipped != meridianFlipped) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				 "need to flip the GP calibration");
			_guider->meridianFlipCalibration(
				snowstar::ControlGuidePort);
		}
		calibration = _guider->getCalibration(
			snowstar::ControlGuidePort);
		if (meridianFlipped != calibration.meridianFlipped) {
			std::string	msg("cannot change flip state for GP");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::logic_error(msg);
		}
		ui->gpFlipBox->setEnabled(true);
	} catch (...) {
		ui->gpFlipBox->setEnabled(false);
	}
}

void	guidercontrollerwidget::gpCalibrationChanged() {
	checkGPFlipped();
}

/**
 * \brief Make sure the AO calibration is properly flipped
 */
void	guidercontrollerwidget::checkAOFlipped() {
	bool	meridianFlipped = (ui->aoFlipBox->checkState() == Qt::Checked);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check whether AO cal is %sflipped",
		(meridianFlipped) ? "" : "not ");
	try {
		snowstar::Calibration	calibration
			= _guider->getCalibration(
				snowstar::ControlAdaptiveOptics);
		if (calibration.meridianFlipped != meridianFlipped) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				 "need to flip the AO calibration");
			_guider->meridianFlipCalibration(
				snowstar::ControlAdaptiveOptics);
		}
		ui->aoFlipBox->setEnabled(true);
	} catch (...) {
		ui->aoFlipBox->setEnabled(false);
	}
}

void	guidercontrollerwidget::aoCalibrationChanged() {
	checkAOFlipped();
}

void	guidercontrollerwidget::showMore(QWidget *parent) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "show a pop menu");
	QMenu	popupmenu("More...", parent);

	QAction	actionRefresh(QString("Refresh"), parent);
	popupmenu.addAction(&actionRefresh);
	connect(&actionRefresh, SIGNAL(triggered()),
		this, SLOT(refreshClicked()));

	QAction	actionDatabase(QString("Track Database"), parent);
	popupmenu.addAction(&actionDatabase);
	connect(&actionDatabase, SIGNAL(triggered()),
		this, SLOT(selectTrack()));

	QAction	actionDECBacklash(QString("DEC Backlash"), parent);
	popupmenu.addAction(&actionDECBacklash);
	connect(&actionDECBacklash, SIGNAL(triggered()),
		this, SLOT(backlashDECClicked()));

	QAction	actionRABacklash(QString("RA Backlash"), parent);
	popupmenu.addAction(&actionRABacklash);
	connect(&actionRABacklash, SIGNAL(triggered()),
		this, SLOT(backlashRAClicked()));

	popupmenu.exec(parent->mapToGlobal(QPoint(0,0)));
}

void	guidercontrollerwidget::showMoreMenu() {
	showMore(ui->moreButton);
}

void	guidercontrollerwidget::refreshClicked() {
	if (!_guider) {
		return;
	}
	_guider->refreshParameters();
}

} // namespade snowgui
