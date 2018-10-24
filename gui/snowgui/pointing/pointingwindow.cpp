/*
 * pointingwindow.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "pointingwindow.h"
#include "ui_pointingwindow.h"
#include <AstroDebug.h>
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Construct a pointingwindow
 */
pointingwindow::pointingwindow(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::pointingwindow) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a pointingwindow");
	ui->setupUi(this);

	// set up the image display widget
	ui->finderImageWidget->setInfoVisible(true);
        ui->finderImageWidget->setRectangleSelectionEnabled(false);
        ui->finderImageWidget->setPointSelectionEnabled(true);

	ui->guiderImageWidget->setInfoVisible(true);
        ui->guiderImageWidget->setRectangleSelectionEnabled(false);
        ui->guiderImageWidget->setPointSelectionEnabled(true);

	ui->imagerImageWidget->setInfoVisible(true);
        ui->imagerImageWidget->setRectangleSelectionEnabled(false);
        ui->imagerImageWidget->setPointSelectionEnabled(true);

	ui->tabWidget->setTabEnabled(0, true);
	ui->tabWidget->setTabEnabled(1, false);
	ui->tabWidget->setTabEnabled(2, false);
	ui->tabWidget->setTabEnabled(3, false);

	// set up the imager
	//ui->imagercontrollerWidget->hideSubframe(true);

	// default setting for the telescope orientation
	_west = true;

	// send new images around
	connect(ui->ccdcontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		this,
		SLOT(newImage(astro::image::ImagePtr)));
	connect(ui->ccdcontrollerWidget,
		SIGNAL(ccddataSelected(ccddata)),
		this,
		SLOT(ccddataSelected(ccddata)));

	// handle the case when a new image is received
	connect(ui->finderImageWidget,
		SIGNAL(pointSelected(astro::image::ImagePoint)),
		this,
		SLOT(finderPointSelected(astro::image::ImagePoint)));
	connect(ui->guiderImageWidget,
		SIGNAL(pointSelected(astro::image::ImagePoint)),
		this,
		SLOT(guiderPointSelected(astro::image::ImagePoint)));
	connect(ui->imagerImageWidget,
		SIGNAL(pointSelected(astro::image::ImagePoint)),
		this,
		SLOT(imagerPointSelected(astro::image::ImagePoint)));

	// when the telesope points to a  new direction, update the
	// star chart
	connect(ui->mountcontrollerWidget,
		SIGNAL(telescopeChanged(astro::RaDec)),
		ui->chartWidget,
		SLOT(directionChanged(astro::RaDec)));
	connect(ui->mountcontrollerWidget,
		SIGNAL(stateChanged(astro::device::Mount::state_type)),
		ui->chartWidget,
		SLOT(stateChanged(astro::device::Mount::state_type)));
	connect(ui->mountcontrollerWidget,
		SIGNAL(orientationChanged(bool)),
		ui->chartWidget,
		SLOT(orientationChanged(bool)));
	connect(ui->chartWidget,
		SIGNAL(pointSelected(astro::RaDec)),
		ui->mountcontrollerWidget,
		SLOT(targetChanged(astro::RaDec)));

	// handle target changes and corrections
	connect(this,
		SIGNAL(targetChanged(astro::RaDec)),
		ui->mountcontrollerWidget,
		SLOT(targetChanged(astro::RaDec)));
	connect(ui->mountcontrollerWidget,
		SIGNAL(radecCorrection(astro::RaDec,bool)),
		ui->guideportcontrollerWidget,
		SLOT(radecCorrection(astro::RaDec,bool)));
	connect(ui->mountcontrollerWidget,
		SIGNAL(orientationChanged(bool)),
		this,
		SLOT(orientationChanged(bool)));

	// handle the resolution signals
	connect(ui->ccdcontrollerWidget, SIGNAL(imagerResolution(astro::Angle)),
		ui->chartWidget, SLOT(imagerResolution(astro::Angle)));
	connect(ui->ccdcontrollerWidget, SIGNAL(finderResolution(astro::Angle)),
		ui->chartWidget, SLOT(finderResolution(astro::Angle)));
	connect(ui->ccdcontrollerWidget, SIGNAL(guiderResolution(astro::Angle)),
		ui->chartWidget, SLOT(guiderResolution(astro::Angle)));
}

/**
 * \brief Destroy a pointing window
 */
pointingwindow::~pointingwindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy pointingwindow");
	delete ui;
}

/**
 * \brief Set up the instrument
 */
void	pointingwindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	ui->ccdcontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->coolercontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->focusercontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->filterwheelcontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->guideportcontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->adaptiveopticscontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->mountcontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
}

/**
 * \brief Main thread initializations
 */
void	pointingwindow::setupComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument setup complete");
	setAppname("Pointing");
}

/**
 * \brief New image received
 */
void    pointingwindow::newImage(astro::image::ImagePtr _image) {
        debug(LOG_DEBUG, DEBUG_LOG, 0, "new image received, offer for saving");
        sendImage(_image, std::string("pointing"));

	// find out which imagedisplaywidget will receive the image
	switch (_ccddata.type()) {
	case snowstar::InstrumentFinderCCD:
		ui->finderImageWidget->setImage(_image);
		ui->tabWidget->setTabEnabled(1, true);
		ui->tabWidget->setCurrentIndex(1);
		_finder_direction = ui->mountcontrollerWidget->current();
		_finder_ccddata = _ccddata;
		_finder_binning = Binning(_image);
		break;
	case snowstar::InstrumentGuiderCCD:
		ui->guiderImageWidget->setImage(_image);
		ui->tabWidget->setTabEnabled(2, true);
		ui->tabWidget->setCurrentIndex(2);
		_guider_direction = ui->mountcontrollerWidget->current();
		_guider_ccddata = _ccddata;
		_guider_binning = Binning(_image);
		break;
	case snowstar::InstrumentCCD:
		ui->imagerImageWidget->setImage(_image);
		ui->tabWidget->setTabEnabled(3, true);
		ui->tabWidget->setCurrentIndex(3);
		_imager_direction = ui->mountcontrollerWidget->current();
		_imager_ccddata = _ccddata;
		_imager_binning = Binning(_image);
		break;
	default:
		// ignored
		break;
	}
}

/**
 * \brief handle window close event
 *
 * This event handler makes sure the window is destroyed when it is closed
 */
void    pointingwindow::closeEvent(QCloseEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "allow deletion");
	sendImage(astro::image::ImagePtr(NULL), std::string());
	deleteLater();
}

/**
 * \brief handle new point selection
 *
 * \param p		p image point to convert to coordinates
 * \param radec		RA/DEC of the center of the image
 * \param _ccd		data about the ccd (pixel size and focal length)
 * \param binning	binning parametrs
 */
void	pointingwindow::pointSelected(astro::image::ImagePoint p,
		const astro::RaDec& radec, const ccddata& _ccd,
		const Binning& binning) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"point %s selected, ccd data %s, binning %s",
		p.toString().c_str(), _ccd.toString().c_str(),
		binning.toString().c_str());
	// get the current coordinates from the mount
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current position: %s",
		radec.toString().c_str());

	// binning correction
	p = p * binning;

	// compute angular resolution
	astro::Angle	angular_resolution(_ccd.ccdinfo().pixelwidth /
				_ccd.focallength());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolution: %.4f",
		angular_resolution.degrees());
	astro::ImageCoordinates	coord(radec, angular_resolution,
					_ccd.azimut(), false);
	// XXX mirror is initialized to false, this will change

	// calculate the new target, this first means that we need the
	// the change relative to the center
	astro::image::ImagePoint	center
		= snowstar::convert(_ccd.ccdinfo().size).center();
	astro::image::ImagePoint	offset = p - center;

	// XXX there is probably something wrong here:
	// XXX when on the east side, we have to invert everything, but we
	// XXX want to compute the correction, so this gives an additional
	// XXX minus sign
	if (!_west) {
		offset = astro::image::ImagePoint(-offset.x(), -offset.y());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s",
		offset.toString().c_str());

	// XXX note that in this, we have not taken into account yet that in
	// XXX the prime focus, the image may be upside down, so we have to
	// XXX mirror it, but this can be down using the mirror function of
	// XXX the ImageCoordinates class (instance coord). Currently we
	// XXX initialize mirror to false (see above)

	// send the new target to the mount controller widget
	astro::RaDec	target = coord(offset);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new target: %s",
		target.toString().c_str());
	emit targetChanged(target);
}

/**
 * \brief direct to a position selected on the finder image
 */
void	pointingwindow::finderPointSelected(astro::image::ImagePoint p) {
	pointSelected(p, _finder_direction, _finder_ccddata, _finder_binning);
}

/**
 * \brief direct to a position selected on the guider image
 */
void	pointingwindow::guiderPointSelected(astro::image::ImagePoint p) {
	pointSelected(p, _guider_direction, _guider_ccddata, _guider_binning);
}

/**
 * \brief direct to a position selected on the main image
 */
void	pointingwindow::imagerPointSelected(astro::image::ImagePoint p) {
	pointSelected(p, _imager_direction, _imager_ccddata, _imager_binning);
}

/**
 * \brief handle a new data record for the ccd
 */
void	pointingwindow::ccddataSelected(ccddata d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a new CCDdata record: %s",
		d.toString().c_str());
	_ccddata = d;
}

/**
 * \brief Handle meridian flip of the telescope
 */
void	pointingwindow::orientationChanged(bool west) {
	_west = west;
}

} // namespace snowgui
