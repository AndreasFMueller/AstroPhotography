/*
 * guidercontrollerwidget.cpp -- implementation of the guider controller
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guidercontrollerwidget.h"
#include "ui_guidercontrollerwidget.h"
#include <QTimer>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <AstroCamera.h>
#include <algorithm>

using namespace astro::camera;
using namespace astro::image;

namespace snowgui {

/**
 * \brief Constructor for the guidercontrollerwidget
 */
guidercontrollerwidget::guidercontrollerwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::guidercontrollerwidget) {
	ui->setupUi(this);

	_guiderdescriptor.ccdIndex = 0;
	_guiderdescriptor.guiderportIndex = 0;
	_guiderdescriptor.adaptiveopticsIndex = 0;

	// adding items to the tracking method combo box
	ui->methodBox->addItem(QString("Star"));
	ui->methodBox->addItem(QString("Phase"));
	ui->methodBox->addItem(QString("Gradient"));
	ui->methodBox->addItem(QString("Laplace"));
	ui->methodBox->addItem(QString("Large"));
	connect(ui->methodBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(methodChanged(int)));

	// connections for other GUI elements
	connect(ui->gpupdateintervalSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(gpupdateintervalChanged(double)));
	connect(ui->aoupdateintervalSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(aoupdateintervalChanged(double)));
	connect(ui->windowradiusSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(windowradiusChanged(double)));
	_windowradius = 32;

	connect(ui->guideButton, SIGNAL(clicked()),
		this, SLOT(startGuiding()));

	// some other fields
	_previousstate = snowstar::GuiderIDLE;
	statusTimer = new QTimer;
	statusTimer->setInterval(100);
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	_gpupdateinterval = 10;
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
	_guiderfactory = snowstar::GuiderFactoryPrx::checkedCast(gbase);

	// now build a GuiderDescriptor for the guider
	_guiderdescriptor.instrumentname = _instrument.name();
	_guiderdescriptor.ccdIndex = 0;
	_guiderdescriptor.guiderportIndex = 0;
	_guiderdescriptor.adaptiveopticsIndex = 0;

	// set up the guider
	setupGuider();
}

/**
 * \brief Setup a guider
 */
void	guidercontrollerwidget::setupGuider() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up the guider %s|%d|%d|%d",
		_guiderdescriptor.instrumentname.c_str(),
		_guiderdescriptor.ccdIndex, _guiderdescriptor.guiderportIndex,
		_guiderdescriptor.adaptiveopticsIndex);
	statusTimer->stop();

	// get the guider based on the descriptor
	_guider = _guiderfactory->get(_guiderdescriptor);

	// also propagate the information to the calibration widgets
	ui->gpcalibrationWidget->setGuider(snowstar::ControlGuiderPort,
		_guiderdescriptor, _guider, _guiderfactory, this);
	ui->aocalibrationWidget->setGuider(snowstar::ControlAdaptiveOptics,
		_guiderdescriptor, _guider, _guiderfactory, this);

	// get the information from the guider
	ui->methodBox->blockSignals(true);
	switch (_guider->getTrackerMethod()) {
	case snowstar::TrackerUNDEFINED:
	case snowstar::TrackerNULL:
	case snowstar::TrackerSTAR:
		ui->methodBox->setCurrentIndex(0);
		break;
	case snowstar::TrackerPHASE:
		ui->methodBox->setCurrentIndex(1);
		break;
	case snowstar::TrackerDIFFPHASE:
		ui->methodBox->setCurrentIndex(2);
		break;
	case snowstar::TrackerLAPLACE:
		ui->methodBox->setCurrentIndex(3);
		break;
	case snowstar::TrackerLARGE:
		ui->methodBox->setCurrentIndex(4);
		break;
	}
	ui->methodBox->blockSignals(false);
	_exposure = snowstar::convert(_guider->getExposure());
	astro::Point	ps = snowstar::convert(_guider->getStar());
	_star = ImagePoint((int)ps.x(), (int)ps.y());

	// start the timer
	statusTimer->start();
}

/**
 * \brief Destructor for the guidercontrollerwidget
 *
 * The destructor also stops and destroys the timer
 */
guidercontrollerwidget::~guidercontrollerwidget() {
	statusTimer->stop();
	delete statusTimer;
	delete ui;
}

/**
 * \brief Set the exposure to use for the guider
 */
void	guidercontrollerwidget::setExposure(astro::camera::Exposure exposure) {
	_exposure = exposure;
	try {
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
 * \brief Select the CCD
 *
 * only GuiderCCD devices are considered
 */
void	guidercontrollerwidget::setCcd(int index) {
	_guiderdescriptor.ccdIndex = index;
	setupGuider();
}

/**
 * \brief Select the Guiderport
 */
void	guidercontrollerwidget::setGuiderport(int index) {
	_guiderdescriptor.guiderportIndex = index;
	setupGuider();
}

/**
 * \brief Select the adaptive optics 
 */
void	guidercontrollerwidget::setAdaptiveoptics(int index) {
	_guiderdescriptor.adaptiveopticsIndex = index;
	setupGuider();
}

/**
 * \brief Set up the tracker
 */
void	guidercontrollerwidget::setupTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window radius: %f", _windowradius);
	// get the current exposure
	Exposure	exposure = snowstar::convert(_guider->getExposure());

	// compute the window for exposures
	ImagePoint	origin(_star.x() - _windowradius,
				_star.y() - _windowradius);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: %s", origin.toString().c_str());
	ImageSize	size(2 * _windowradius, 2 * _windowradius);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %s", size.toString().c_str());
	exposure.frame(ImageRectangle(origin, size));
	_guider->setExposure(snowstar::convert(exposure));

#if 0
	// set the star for which tracking should take place
	snowstar::Point	star;
	star.x = _windowradius;
	star.y = _windowradius;
	_guider->setStar(star);
#endif
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
	try {
		setupTracker();
		_guider->startGuiding(_gpupdateinterval, _aoupdateinterval,
			_stepping);
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
	snowstar::GuiderState	state = _guider->getState();
	if (state == _previousstate) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state: %d", (int)state);
	switch (state) {
	case snowstar::GuiderIDLE:
		ui->guideButton->setText(QString("Guiding"));
		ui->guideButton->setEnabled(false);
		ui->gpcalibrationWidget->setEnabled(true);
		ui->aocalibrationWidget->setEnabled(true);
		break;
	case snowstar::GuiderUNCONFIGURED:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(false);
		ui->gpcalibrationWidget->setEnabled(true);
		ui->aocalibrationWidget->setEnabled(true);
		break;
	case snowstar::GuiderCALIBRATING:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(false);
		ui->gpcalibrationWidget->setEnabled(false);
		ui->aocalibrationWidget->setEnabled(false);
		break;
	case snowstar::GuiderCALIBRATED:
		ui->guideButton->setText(QString("Guide"));
		ui->guideButton->setEnabled(true);
		ui->gpcalibrationWidget->setEnabled(true);
		ui->aocalibrationWidget->setEnabled(true);
		break;
	case snowstar::GuiderGUIDING:
		ui->guideButton->setText(QString("Stop Guiding"));
		ui->guideButton->setEnabled(true);
		ui->gpcalibrationWidget->setEnabled(false);
		ui->aocalibrationWidget->setEnabled(false);
		break;
	}
	_previousstate = state;
}

/**
 * \brief Change the tracker method
 */
void	guidercontrollerwidget::methodChanged(int index) {
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
 * \brief update the guiderport change interval
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
void	guidercontrollerwidget::windowradiusChanged(double r) {
	_windowradius = r;
}

} // namespade snowgui
