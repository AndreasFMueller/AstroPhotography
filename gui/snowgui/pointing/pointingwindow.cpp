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
	ui->imageWidget->setInfoVisible(true);
        ui->imageWidget->setRectangleSelectionEnabled(false);
        ui->imageWidget->setPointSelectionEnabled(true);

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

	connect(ui->imageWidget,
		SIGNAL(pointSelected(astro::image::ImagePoint)),
		this,
		SLOT(pointSelected(astro::image::ImagePoint)));
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
	ui->imageWidget->setImage(_image);
        sendImage(_image, std::string("pointing"));
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
void	pointingwindow::pointSelected(astro::image::ImagePoint p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "point %s selected, data %s",
		p.toString().c_str(), _ccddata.toString().c_str());
	// get the current coordinates from the mount
	astro::RaDec	radec = ui->mountcontrollerWidget->current();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current position: %s",
		radec.toString().c_str());

	// compute angular resolution
	astro::Angle	angular_resolution(_ccddata.ccdinfo().pixelwidth /
				_ccddata.focallength());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resolution: %.4f",
		angular_resolution.degrees());
	astro::ImageCoordinates	coord(radec, angular_resolution,
					_ccddata.azimut(), false);

	// calculate the new target
	astro::image::ImagePoint	center = snowstar::convert(_ccddata.ccdinfo().size).center();
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
 * \brief handle a new data record for the ccd
 */
void	pointingwindow::ccddataSelected(ccddata d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a new CCDdata record: %s",
		d.toString().c_str());
	_ccddata = d;
}

} // namespace snowgui
