/*
 * pointingwindow.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "pointingwindow.h"
#include "ui_pointingwindow.h"
#include <AstroDebug.h>

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "point %s selected",
		p.toString().c_str());
}

} // namespace snowgui
