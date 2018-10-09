/*
 * ccdcontrollerwidget.cpp --  ccd controller implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ccdcontrollerwidget.h"
#include "ui_ccdcontrollerwidget.h"
#include <IceConversions.h>
#include <algorithm>
#include <QTimer>
#include <AstroIO.h>
#include <AstroImageops.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <QMessageBox>
#include <HideWidget.h>
#include <HideProgress.h>

using namespace astro::image;
using namespace astro::io;
using namespace astro::camera;

namespace snowgui {

/**
 * \brief Constructor for the CCD controller
 */
ccdcontrollerwidget::ccdcontrollerwidget(QWidget *parent) :
	InstrumentWidget(parent), ui(new Ui::ccdcontrollerwidget) {
	// setup user interface components
	ui->setupUi(this);
	_hideprogress = NULL;
	_hide = NULL;
	_imageretriever = NULL;

	// register some types
	qRegisterMetaType<astro::camera::Exposure>("astro::camera::Exposure");
	qRegisterMetaType<astro::image::ImagePtr>("astro::image::ImagePtr");
	qRegisterMetaType<snowstar::ImagePrx>("snowstar::ImagePrx");
	qRegisterMetaType<snowstar::ExposureState>("snowstar::ExposureState");

	// install all internal connections
	ui->ccdSelectionBox->blockSignals(true);
	connect(ui->ccdSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(ccdChanged(int)));

	connect(ui->exposureSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(guiChanged()));
	connect(ui->binningSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(guiChanged()));
	connect(ui->shutterOpenBox, SIGNAL(toggled(bool)),
		this, SLOT(guiChanged()));
	connect(ui->purposeBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(guiChanged()));

	connect(ui->captureButton, SIGNAL(clicked()),
		this, SLOT(captureClicked()));
	connect(ui->cancelButton, SIGNAL(clicked()),
		this, SLOT(cancelClicked()));
	connect(ui->streamButton, SIGNAL(clicked()),
		this, SLOT(streamClicked()));

	connect(ui->frameFullButton, SIGNAL(clicked()),
		this, SLOT(guiChanged()));

	// setup and connect the timer
	ourexposure = false;
	_guiderccdonly = false;
	_nosubframe = false;
	_nobuttons = false;
	_imageproxyonly = false;

	// make sure the widget cannot be used unless a CCD is configured
	ui->ccdInfo->setEnabled(false);
	ui->frameWidget->setEnabled(false);
	ui->buttonArea->setEnabled(false);

	// start the state monitoring thread
	_statemonitoringthread = new StateMonitoringThread(this);
	connect(_statemonitoringthread,
		SIGNAL(stateChanged(snowstar::ExposureState)),
		this, SLOT(statusUpdate(snowstar::ExposureState)));
	_statemonitoringthread->start();

	// make sure no signals are emitted during setup
	ui->ccdSelectionBox->blockSignals(true);
}

/**
 * \brief Common instrument setup
 *
 * This method is called to add instrument information. The default
 * constructor called by the Designer generated code cannot include
 * this information, so we supply it later..
 */
void	ccdcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"begin ccdcontrollerwidget::instrumentSetup()");

	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// initialize the parameters we will need later
	double	focallength = 1.0;
	double	azimuth = 0;
	double	guiderfocallength = 1.0;
	double	guiderazimuth = 0;
	double	finderfocallength = 1.0;
	double	finderazimuth = 0;

	// get the parameters wie are going to need later;
	if (instrument.hasProperty("focallength")) {
		focallength = instrument.doubleProperty("focallength");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focallength = %.3f",
			focallength);
	}
	if (instrument.hasProperty("azimuth")) {
		azimuth = instrument.doubleProperty("azimuth")
			* M_PI / 180;
	}
	if (instrument.hasProperty("guiderfocallength")) {
		guiderfocallength
			= instrument.doubleProperty("guiderfocallength");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderfocallength = %.3f",
			guiderfocallength);
	}
	if (instrument.hasProperty("guiderazimuth")) {
		guiderazimuth = instrument.doubleProperty("guiderazimuth")
			* M_PI / 180;
	}
	if (instrument.hasProperty("finderfocallength")) {
		finderfocallength
			= instrument.doubleProperty("finderfocallength");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "finderfocallength = %.3f",
			finderfocallength);
	}
	if (instrument.hasProperty("finderazimuth")) {
		finderazimuth = instrument.doubleProperty("finderazimuth")
			* M_PI / 180;
	}

	// read information about CCDs available on this instrument, and 
	// remember the first ccd you can find
	int	index = 0;
	if (!_guiderccdonly) {
		// add the imaging CCDs
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding guider ccds");
		while (_instrument.has(snowstar::InstrumentCCD, index)) {
			try {
				snowstar::CcdPrx	ccd
					= _instrument.ccd(index);
				std::string	sn = instrument.displayname(
					snowstar::InstrumentCCD, index,
					serviceobject.name());
				ccddata	d(snowstar::InstrumentCCD, index,
					focallength, azimuth, sn);
				d.ccdinfo(ccd->getInfo());
				debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s",
					d.toString().c_str());
				_ccddata.push_back(d);
				if (!_ccd) {
					_ccd = ccd;
					_current_ccddata = d;
				}
			} catch (const std::exception& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"ignoring imaging ccd %d", index);
			}
			index++;
		}
		// add all finder CCDs
		index = 0;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding finder ccds");
		while (_instrument.has(snowstar::InstrumentFinderCCD, index)) {
			try {
				snowstar::CcdPrx	ccd
					= _instrument.finderccd(index);
				std::string	sn = instrument.displayname(
					snowstar::InstrumentFinderCCD, index,
					serviceobject.name());
				sn = sn + " (finder)";
				ccddata	d(snowstar::InstrumentFinderCCD, index,
					finderfocallength, finderazimuth, sn);
				d.ccdinfo(ccd->getInfo());
				_ccddata.push_back(d);
				debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s",
					d.toString().c_str());
				if (!_ccd) {
					_ccd = ccd;
					_current_ccddata = d;
				}
			} catch (const std::exception& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"ignoring finder ccd %d", index);
			}
			index++;
		}
		index = 0;
	}
	// add all guider CCDs
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding imaging ccds");
	while (_instrument.has(snowstar::InstrumentGuiderCCD, index)) {
		try {
			snowstar::CcdPrx	ccd
				= _instrument.guiderccd(index);
			std::string	sn = instrument.displayname(
				snowstar::InstrumentGuiderCCD, index,
				serviceobject.name());
			sn = sn + " (guider)";
			ccddata	d(snowstar::InstrumentGuiderCCD, index,
				guiderfocallength, guiderazimuth, sn);
			d.ccdinfo(ccd->getInfo());
			_ccddata.push_back(d);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s",
				d.toString().c_str());
			if (!_ccd) {
				_ccd = ccd;
				_current_ccddata = d;
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring guider ccd %d",
				index);
		}
		index++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found total of %d ccds",
		_ccddata.size());
}

/**
 * \brief setup stuff to be executed on the main thread
 */
void	ccdcontrollerwidget::setupComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setup complete");
	// add the entries to the menu
	std::vector<ccddata>::const_iterator	i;
	for (i = _ccddata.begin(); i != _ccddata.end(); i++) {
		std::string	sn = i->name();
		ui->ccdSelectionBox->addItem(QString(sn.c_str()));
	}

	// add additional information about this ccd
	setupCcd();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"end ccdcontrollerwidget::instrumentSetup()");
	ui->ccdSelectionBox->blockSignals(false);
	emit ccdSelected(0);
	emit ccddataSelected(_current_ccddata);
}

/**
 * \brief Destroy the CCD controller
 */
ccdcontrollerwidget::~ccdcontrollerwidget() {
	delete _statemonitoringthread;
	delete ui;
}

/**
 * \brief Read information from the ccd and show it
 */
void	ccdcontrollerwidget::setupCcd() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setupCcd() begin");
	// we set the previous state to idle, but if that is not correct, then
	// then the first status update will fix it.
	ui->captureButton->setEnabled(true);
	ui->cancelButton->setEnabled(false);
	ui->streamButton->setEnabled(true);

	// make sure no signals are sent while setting up the CCD
	ui->binningSelectionBox->setEnabled(false);
	ui->binningSelectionBox->blockSignals(true);

	// remove all entry from the binning mode combobox
	while (ui->binningSelectionBox->count() > 0) {
		ui->binningSelectionBox->removeItem(0);
	}

	// propagate the information from the ccdinfo structure
	if (_ccd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get info of the CCD");
		_ccdinfo = _ccd->getInfo();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got info of the CCD");
		QComboBox	*binBox = ui->binningSelectionBox;
		CcdInfo	ccdinfo(snowstar::convert(_ccdinfo));
		std::for_each(ccdinfo.modes().begin(), ccdinfo.modes().end(),
			[binBox](const Binning& mode) {
				std::string	m = astro::stringprintf("%dx%d",
					mode.x(), mode.y());
				binBox->addItem(QString(m.c_str()));
			}
		);

		// get pixel size information from Ccd
		std::string	ccdinfotext 
			= astro::stringprintf("%d x %d (%.1fµm x %.1fµm)",
				ccdinfo.size().width(), ccdinfo.size().height(),
				1000000 * ccdinfo.pixelwidth(),
				1000000 * ccdinfo.pixelheight());
		ui->sizeInfoField->setText(QString(ccdinfotext.c_str()));

		// use the frame size as the default rectangle
		displayFrame(ImageRectangle(ccdinfo.size()));

		// set the exposure time range
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure time range: %f - %f",
			ccdinfo.minexposuretime(), ccdinfo.maxexposuretime());
		ui->exposureSpinBox->setMinimum(ccdinfo.minexposuretime());
		ui->exposureSpinBox->setMaximum(ccdinfo.maxexposuretime());
		int	dec = round(log10(ccdinfo.minexposuretime()));
		if (dec > 0) { dec = 0; } else { dec = -dec; }
		ui->exposureSpinBox->setDecimals(dec);

		// query the status
		statusUpdate(_ccd->exposureStatus());
	}

	// now reenable signals
	ui->binningSelectionBox->blockSignals(false);
	ui->binningSelectionBox->setEnabled(true);

	// enable everything
	ui->ccdInfo->setEnabled(true);
	ui->frameWidget->setEnabled(true);
	ui->buttonArea->setEnabled(true);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setupCcd() end");
}

/**
 * \brief Display the settings in the argument exposure 
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayExposure(Exposure e) {
	displayBinning(e.mode());
	displayExposureTime(e.exposuretime());
	displayPurpose(e.purpose());
	displayShutter(e.shutter());
}

/**
 * \brief This is the slot to call when setting an exposure from the outside
 *
 * This method ensures that all the GUI elements are updated, and then sends 
 * the exposureChanged() signal to whatever widget is connected to it.
 */
void	ccdcontrollerwidget::setExposure(Exposure e) {
	if (_exposure != e) {
		return;
	}
	displayExposure(_exposure);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display new frame settings
 *
 * This method ensures that the GUI element and the internal state are in
 * sync, but it does not send any signals.
 */
void	ccdcontrollerwidget::displayFrame(ImageRectangle r) {
	// is the rectangle contained in the ccd
	if (!snowstar::convert(_ccdinfo).size().bounds(r)) {
		return;
	}
	_exposure.frame(r);
	ui->frameSizeWidth->blockSignals(true);
	ui->frameSizeWidth->setText(
		QString(astro::stringprintf("%d", r.size().width()).c_str()));
	ui->frameSizeWidth->blockSignals(false);
	ui->frameSizeHeight->blockSignals(true);
	ui->frameSizeHeight->setText(
		QString(astro::stringprintf("%d", r.size().height()).c_str()));
	ui->frameSizeHeight->blockSignals(false);
	ui->frameOriginX->blockSignals(true);
	ui->frameOriginX->setText(
		QString(astro::stringprintf("%d", r.origin().x()).c_str()));
	ui->frameOriginX->blockSignals(false);
	ui->frameOriginY->blockSignals(true);
	ui->frameOriginY->setText(
		QString(astro::stringprintf("%d", r.origin().y()).c_str()));
	ui->frameOriginY->blockSignals(false);
}

/**
 * \brief Change the subframe rectangle
 *
 * This slot sends the exposureChanged signal
 */
void	ccdcontrollerwidget::setFrame(ImageRectangle r) {
	if (_exposure.frame() == r) {
		return;
	}
	displayFrame(r);
	emit exposureChanged(_exposure);
}

/**
 * \brief set the frame
 *
 * This method converts the rectangle to CCD coordinates. Note that
 * only this controller knows about the binning mode, so it has to compute
 * unbinned coordinates.
 */
void	ccdcontrollerwidget::setSubframe(ImageRectangle r) {
	ImagePoint	origin = r.origin() + _exposure.frame().origin();
	ImageRectangle	newrectangle
		= ImageRectangle(origin, r.size()) * _exposure.mode();
	setFrame(newrectangle);
}

/**
 * \brief Display the new binning mode
 *
 * This method does not send signals
 */
void	ccdcontrollerwidget::displayBinning(Binning b) {
	// is binning mode supported by this camera?
	if (!snowstar::convert(_ccdinfo).modes().permits(b)) {
		// XXX should we send an exeption here?
		return;
	}
	_exposure.mode(b);
	ui->binningSelectionBox->blockSignals(true);
	QString	modestring(b.toString().c_str());
	for (int i = 0; i < ui->binningSelectionBox->count(); i++) {
		if (modestring == ui->binningSelectionBox->itemText(i)) {
			ui->binningSelectionBox->setCurrentIndex(i);
			break;
		}
	}
	ui->binningSelectionBox->blockSignals(false);
}

/**
 * \brief private mtethod to get the binning mode from the selected item index
 */
Binning	ccdcontrollerwidget::getBinning(int index) {
	if ((index >= ui->binningSelectionBox->count()) && (index < 0)) {
		std::string	msg = astro::stringprintf("invalid binning "
			"index: %d", index);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	Binning	b(ui->binningSelectionBox->itemText(index).toLatin1().data());
	return b;
}

/**
 * \brief Display the binning mode based on the index
 */
void	ccdcontrollerwidget::displayBinning(int index) {
	displayBinning(getBinning(index));
}

/**
 * \brief Set the binning mode
 *
 * This method displays the new binning mode (if it is acceptable, i.e.
 * supported by the CCD in use), and then sends the exposureChanged signal.
 */
void	ccdcontrollerwidget::setBinning(Binning b) {
	if (_exposure.mode() == b) {
		return;
	}
	displayBinning(b);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display the exposure time
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayExposureTime(double t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new exposure time: %.3f", t);
	_exposure.exposuretime(t);
	ui->exposureSpinBox->blockSignals(true);
	ui->exposureSpinBox->setValue(t);
	ui->exposureSpinBox->blockSignals(false);
}

/**
 * \brief Set the exposure time
 */
void	ccdcontrollerwidget::setExposureTime(double t) {
	displayExposureTime(t);
	emit exposureChanged(_exposure);
}

/**
 * \brief Get the purpose from the menu index
 */
Exposure::purpose_t	ccdcontrollerwidget::getPurpose(int index) {
	switch (index) {
	case 0:	return Exposure::light;
	case 1:	return Exposure::dark;
	case 2:	return Exposure::flat;
	case 3:	return Exposure::bias;
	case 4:	return Exposure::test;
	case 5:	return Exposure::guide;
	case 6:	return Exposure::focus;
	case 7:	return Exposure::flood;
	default:
		break;
	}
	std::string	msg = astro::stringprintf("invalid purpose index: %d",
		index);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::range_error(msg);
}

/**
 * \brief Display the new purpose
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayPurpose(Exposure::purpose_t p) {
	_exposure.purpose(p);
	ui->purposeBox->blockSignals(true);
	ui->purposeBox->setCurrentIndex((int)p);
	ui->purposeBox->blockSignals(false);
}

/**
 * \brief Set a new purpose for the next exposure
 *
 * This method sets the new purpose and then sends the exposureChanged()
 * signal.
 */
void	ccdcontrollerwidget::setPurpose(Exposure::purpose_t p) {
	if (_exposure.purpose() == p) {
		return;
	}
	displayPurpose(p);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display the new shutter state
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayShutter(Shutter::state s) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutter state: %s",
		(_exposure.shutter() == Shutter::OPEN) ? "open" : "closed");
	_exposure.shutter(s);
	ui->shutterOpenBox->blockSignals(true);
	if (_exposure.shutter() == Shutter::OPEN) {
		ui->shutterOpenBox->setChecked(true);
	} else {
		ui->shutterOpenBox->setChecked(false);
	}
	ui->shutterOpenBox->blockSignals(false);
}

/**
 * \brief Set new Shutter settings
 *
 * This method changes the shutter setting of the Exposure and then
 * sends the exposureChanged() signal
 */
void	ccdcontrollerwidget::setShutter(Shutter::state s) {
	if (s == _exposure.shutter()) {
		return;
	}
	displayShutter(s);
	emit exposureChanged(_exposure);
}

/**
 * \brief internal Slot activate when a gui element changes
 */
void	ccdcontrollerwidget::guiChanged() {
	if (sender() == ui->binningSelectionBox) {
		displayBinning(getBinning(
			ui->binningSelectionBox->currentIndex()));
	}
	if (sender() == ui->exposureSpinBox) {
		displayExposureTime(ui->exposureSpinBox->value());
	}
	if (sender() == ui->purposeBox) {
		displayPurpose(getPurpose(ui->purposeBox->currentIndex()));
	}
	if (sender() == ui->shutterOpenBox) {
		displayShutter(ui->shutterOpenBox->isChecked()
			? Shutter::OPEN : Shutter::CLOSED);
	}
	if (sender() == ui->frameFullButton) {
		displayFrame(ImageRectangle(snowstar::convert(_ccdinfo).size()));
	}
	emit exposureChanged(_exposure);
}


/**
 * \brief Slot to handle a new image
 */
void	ccdcontrollerwidget::setImage(ImagePtr image) {
	std::lock_guard<std::recursive_mutex>	_lock(_mutex);
	_image = image;
	emit imageReceived(_image);
}

/**
 * \brief Slot to handle a change of the selected CCD
 */
void	ccdcontrollerwidget::ccdChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "CCD changed: %d from %d", index,
		_ccddata.size());
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking index = %d", index);
		_current_ccddata = _ccddata[index];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s",
			_current_ccddata.toString().c_str());
		switch (_current_ccddata.type()) {
		case snowstar::InstrumentCCD:
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"getting instrument ccd %d",
				_current_ccddata.index());
			_ccd = _instrument.ccd(_current_ccddata.index());
			break;
		case snowstar::InstrumentFinderCCD:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "getting finder ccd %d",
				_current_ccddata.index());
			_ccd = _instrument.finderccd(_current_ccddata.index());
			break;
		case snowstar::InstrumentGuiderCCD:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "getting guider ccd %d",
				_current_ccddata.index());
			_ccd = _instrument.guiderccd(_current_ccddata.index());
			break;
		default:
			debug(LOG_ERR, DEBUG_LOG, 0, "bad type, ignored");
			throw std::runtime_error("internal error: bad type");
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get ccd[%d]: %s",
			index, x.what());
		ccdFailed(x);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd selected, setting up");
        setupCcd();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "emitting ccdSelected(%d)", index);
	emit ccdSelected(index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "emitting ccddataSelected(%d)", index);
	emit ccddataSelected(_current_ccddata);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the current state");
}

/**
 * \brief Slot to handle click on the "Capture" button
 */
void	ccdcontrollerwidget::captureClicked() {
	// first find out what state the ccd is in
	try {
		if (_ccd->exposureStatus() == snowstar::EXPOSED) {
			// make it our exposure
			ourexposure = true;
			retrieveImageStart();
			return;
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get exposure status: %s",
			x.what());
	}

	// now try to start a new exposure
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"start exposure with time=%.3f, shutter = %s",
			_exposure.exposuretime(),
			(_exposure.shutter() == Shutter::OPEN)
				? "open" : "closed");
		try {
			_ccd->startExposure(snowstar::convert(_exposure));
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"cannot start exposure: %s", x.what());
			ccdFailed(x);
			return;
		}
		ourexposure = true;
		ui->captureButton->setEnabled(false);
		ui->streamButton->setEnabled(false);
		ui->cancelButton->setEnabled(true);

		// start the hide widget which will tell the user that
		// something is going on
		_hideprogress = new HideProgress(_exposure.exposuretime(), this);
		_hideprogress->setGeometry(0, height() - 10, width(), 10);
		_hideprogress->setVisible(true);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot start exposure");
	}
}

/**
 * \brief Slot tohandle click on the "Cancel" button
 */
void	ccdcontrollerwidget::cancelClicked() {
	try {
		_ccd->cancelExposure();
	} catch (const snowstar::BadState& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad state: %s", x.what());
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot cancel: %s", x.what());
		ccdFailed(x);
		return;
	}
}

/**
 * \brief Slto to handle click on the "Stream" button
 */
void	ccdcontrollerwidget::streamClicked() {
	try {
		_ccd->startStream(snowstar::convert(_exposure));
	} catch (const snowstar::BadState& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad state for stream: %s",
			x.what());
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot start stream: %s",
			x.what());
		ccdFailed(x);
		return;
	}
}

/**
 * \brief initiate the image retrieval thread
 */
void	ccdcontrollerwidget::retrieveImageStart() {
	// make sure we are actually accessing our own exposure
	if (!ourexposure) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not our exposure, giving up");
		return;
	}
	ourexposure = false;

	// this seams to be our exposure
	if (_hideprogress) {
		delete _hideprogress;
		_hideprogress = NULL;
	}

	// acquire a lock
	std::lock_guard<std::recursive_mutex>	_lock(_mutex);

	// make sure we can start the image retriever
	if (_imageretriever) {
		throw std::runtime_error("an image retriever is already running");
	}

	// prepare the retrieval thread
	_imageretriever = new ImageRetrieverThread(this);
	connect(_imageretriever, SIGNAL(finished()),
		this, SLOT(retrieveImageComplete()));
	connect(_imageretriever, SIGNAL(failed(QString)),
		this, SLOT(retrieveImageFailed(QString)));

	// prepare the hide widget
	_hide = new HideWidget( QString("retrieving image..."), this);
	_hide->setGeometry(0, 0, width(), height());
	_hide->setVisible(true);

	// now start working
	_imageretriever->start();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image retriever thread started");
}

/**
 * \brief Do the main work for retrievin an image
 */
void	ccdcontrollerwidget::retrieveImageWork() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start work thread to retrieve image");
	try {
		// now assign the data to the state variables, but do this
		// only after a lock is established, because otherwise
		// the data may not be consistent
		std::lock_guard<std::recursive_mutex>	_lock(_mutex);

		// use a separate connection for the image transfer to
		// ensure that the image transfor does not impact the
		// other method calls
		_imageproxy = _ccd->getImage()->ice_connectionId("image");
		if (!_imageproxy->hasMeta("INSTRUME")) {
			snowstar::Metavalue	v;
			v.keyword = "INSTRUME";
			v.value = instrumentname();
			_imageproxy->setMetavalue(v);
		}
		if (_imageproxyonly) {
			emit imageproxyReceived(_imageproxy);
			return;
		}

		// at this point we actually download the image
		ImagePtr	image = snowstar::convert(_imageproxy);

		_image = image;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image frame: %s",
			image->getFrame().toString().c_str());
		_imageexposure = snowstar::convert(_ccd->getExposure());
		_imageproxy->remove();

		// if the image size does not match the size requested, get the
		// subimage
		if (_image->getFrame() != _exposure.frame()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cutting image to %s",
				_exposure.frame().toString().c_str());
			_image = astro::image::ops::cut(_image, _exposure.frame());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image dimensions now %s",
			_image->getFrame().toString().c_str());
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot retrieve "
			"image: exception %s, cause=%s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		emit retrieveImageFailed(QString(msg.c_str()));
	}
}

/**
 * \brief Slot used to signal image retrieval completion to the controller
 */
void	ccdcontrollerwidget::retrieveImageComplete() {
	// turn of the HideWidget
	delete _hide;
	_hide = NULL;
	{
		std::lock_guard<std::recursive_mutex>	_lock(_mutex);
		delete _imageretriever;
		_imageretriever = NULL;
	}
	// signal that the image was received
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image received, emit signal");
	emit imageReceived(_image);
}

void	ccdcontrollerwidget::retrieveImageFailed(QString x) {
	delete _hide;
	_hide = NULL;
	// XXX show a dialog that reports the reason for the failure
}

/**
 * \brief Status update slot
 *
 * This slot is called by the 
 */
void	ccdcontrollerwidget::statusUpdate(snowstar::ExposureState newstate) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state update: newstate=%d", newstate);
	if (!_ccd) {
		return;
	}
	switch (newstate) {
	case snowstar::IDLE:
		if (_hideprogress) {
			delete _hideprogress;
			_hideprogress = NULL;
		}
		ui->captureButton->setText(QString("Capture"));
		ui->captureButton->setEnabled(true);
		ui->cancelButton->setEnabled(false);
		ui->streamButton->setEnabled(true);
		ui->streamButton->setText(QString("Stream"));
		break;
	case snowstar::EXPOSING:
		ui->captureButton->setText(QString("Capture"));
		ui->captureButton->setEnabled(false);
		ui->cancelButton->setEnabled(true);
		ui->streamButton->setEnabled(false);
		break;
	case snowstar::EXPOSED:
		// if we get to this point, then an exposure just completed,
		// and we we should retrieve the image
		if (ourexposure) {
			retrieveImageStart();
			ui->captureButton->setEnabled(false);
		} else {
			ui->captureButton->setText(QString("Get Image"));
			ui->captureButton->setEnabled(true);
		}
		ui->cancelButton->setEnabled(false);
		ui->streamButton->setEnabled(false);
		break;
	case snowstar::CANCELLING:
		ui->captureButton->setText(QString("Capture"));
		ui->captureButton->setEnabled(false);
		ui->cancelButton->setEnabled(false);
		ui->streamButton->setEnabled(false);
		break;
	case snowstar::STREAMING:
		ui->captureButton->setText(QString("Capture"));
		ui->captureButton->setEnabled(false);
		ui->cancelButton->setEnabled(false);
		ui->streamButton->setEnabled(true);
		ui->streamButton->setText(QString("Stop"));
		break;
	}
}

void	ccdcontrollerwidget::hideSubframe(bool sf) {
	_nosubframe = sf;
	ui->frameWidget->setHidden(_nosubframe);
}

void	ccdcontrollerwidget::hideButtons(bool b) {
	_nobuttons = b;
	ui->buttonArea->setHidden(_nobuttons);
}

void	ccdcontrollerwidget::ccdFailed(const std::exception& x) {
	_ccd = NULL;
	ui->ccdInfo->setEnabled(false);
	ui->frameWidget->setEnabled(false);
	ui->buttonArea->setEnabled(false);
	QMessageBox	message;
	message.setText(QString("CCD failed"));
	std::ostringstream	out;
	out << "Communication with the CCD '";
	out << ui->ccdSelectionBox->currentText().toLatin1().data();
	out << "' failed." << std::endl;
	out << "The reason for the failure was: " << x.what() << std::endl;
	out << "The CCD has been disabled and can no longer be used.";
	message.setInformativeText(QString(out.str().c_str()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccdFailed: %s", out.str().c_str());
	message.exec();
}

} // namespace snowgui
