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
	connect(ui->chartWidget,
		SIGNAL(pointSelected(astro::RaDec)),
		ui->mountcontrollerWidget,
		SLOT(targetChanged(astro::RaDec)));
}

/**
 * \brief Destroy a pointing window
 */
pointingwindow::~pointingwindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy pointingwindow");
	delete ui;
}

/**
 * \brief Set up the instru
 */
void	pointingwindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	ui->ccdcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->focusercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->guideportcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->adaptiveopticscontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->mountcontrollerWidget->instrumentSetup(serviceobject, instrument);
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
		break;
	case snowstar::InstrumentGuiderCCD:
		ui->guiderImageWidget->setImage(_image);
		ui->tabWidget->setTabEnabled(2, true);
		ui->tabWidget->setCurrentIndex(2);
		_guider_direction = ui->mountcontrollerWidget->current();
		_guider_ccddata = _ccddata;
		break;
	case snowstar::InstrumentCCD:
		ui->imagerImageWidget->setImage(_image);
		ui->tabWidget->setTabEnabled(3, true);
		ui->tabWidget->setCurrentIndex(3);
		_imager_direction = ui->mountcontrollerWidget->current();
		_imager_ccddata = _ccddata;
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
 */
void	pointingwindow::pointSelected(astro::image::ImagePoint p,
		const astro::RaDec& radec, const ccddata& _ccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "point %s selected, ccd data %s",
		p.toString().c_str(), _ccd.toString().c_str());
	// get the current coordinates from the mount
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current position: %s",
		radec.toString().c_str());

	// compute angular resolution
	astro::Angle	angular_resolution(_ccd.ccdinfo().pixelwidth /
				_ccd.focallength());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolution: %.4f",
		angular_resolution.degrees());
	astro::ImageCoordinates	coord(radec, angular_resolution,
					_ccd.azimut(), false);

	// calculate the new target
	astro::image::ImagePoint	center
		= snowstar::convert(_ccd.ccdinfo().size).center();
	astro::image::ImagePoint	offset = p - center;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s",
		offset.toString().c_str());

	// send the new target to the mount controller widget
	astro::RaDec	target = coord(offset);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new target: %s",
		target.toString().c_str());
	ui->mountcontrollerWidget->setTarget(target);
}

/**
 * \brief direct to a position selected on the finder image
 */
void	pointingwindow::finderPointSelected(astro::image::ImagePoint p) {
	pointSelected(p, _finder_direction, _finder_ccddata);
}

/**
 * \brief direct to a position selected on the guider image
 */
void	pointingwindow::guiderPointSelected(astro::image::ImagePoint p) {
	pointSelected(p, _guider_direction, _guider_ccddata);
}

/**
 * \brief direct to a position selected on the main image
 */
void	pointingwindow::imagerPointSelected(astro::image::ImagePoint p) {
	pointSelected(p, _imager_direction, _imager_ccddata);
}

/**
 * \brief handle a new data record for the ccd
 */
void	pointingwindow::ccddataSelected(ccddata d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a new CCDdata record: %s",
		d.toString().c_str());
	_ccddata = d;
}

} // namespace snowgui
