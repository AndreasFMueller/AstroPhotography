/*
 * takeimagewindow.cpp -- implementation of image taking window
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "takeimagewindow.h"
#include "ui_takeimagewindow.h"
#include <AstroImage.h>
#include <AstroCamera.h>
#include <imagedisplaywidget.h>

using namespace astro::image;
using namespace astro::camera;

namespace snowgui {

/**
 * \brief Create a new takeimagewindow
 */
takeimagewindow::takeimagewindow(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::takeimagewindow) {
	ui->setupUi(this);
	ui->imageWidget->setInfoVisible(true);

	// when the CCD controller receives a new image, we would like to know
	connect(ui->ccdcontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		this,
		SLOT(receiveImage(astro::image::ImagePtr)));

	// when the image widget selects a rectangle, we would like to know
	ui->imageWidget->setRectangleSelectionEnabled(true);
	connect(ui->imageWidget,
		SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
		this,
		SLOT(rectangleSelected(astro::image::ImageRectangle)));
}

/**
 * \brief Destroy the takeimagewindow
 */
takeimagewindow::~takeimagewindow() {
	delete ui;
}

/**
 * \brief setup the instruments
 */
void	takeimagewindow::instrumentSetup(
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

	setAppname("Take images");
}

/**
 * \brief What to do when the CCD controller has received an image
 *
 * This method reads the image from the CCD controller, and copies it
 * to the imageWidget. Furthermore it takes the image exposure and
 * installs it as the exposure for the next image. This is important for
 * cameras that change the rectangle from the one originally specified, 
 * like the SX M26C.
 * 
 * Furthermore it adds a focus point to the focusinghistoryWidget.
 */
void    takeimagewindow::receiveImage(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new %s image received",
		image->size().toString().c_str());
	ui->imageWidget->setImage(image);
	Exposure        imageexposure
		= ui->ccdcontrollerWidget->imageexposure();
	ui->ccdcontrollerWidget->setExposure(imageexposure);
	sendImage(image, std::string("preview"));
}

/**
 * \brief Slot to handle a new rectangle
 *
 * This slot installs the rectangle as a sub frame of the CCD controller
 */
void    takeimagewindow::rectangleSelected(ImageRectangle rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new rectangle: %s",
		rectangle.toString().c_str());
	ui->ccdcontrollerWidget->setSubframe(rectangle);
}

/**
 * \brief Make sure object is destroyed when the window closes
 */
void    takeimagewindow::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui
