/*
 * focusingwindow.cpp -- implementation of the focusing widget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "focusingwindow.h"
#include "ui_focusingwindow.h"
#include <AstroImage.h>
#include <AstroCamera.h>
#include <imagedisplaywidget.h>
#include <FocusPoints.h>

using namespace astro::image;
using namespace astro::camera;

namespace snowgui {

/**
 * \brief Create a new focusing widget
 */
focusingwindow::focusingwindow(QWidget *parent)
	: snowgui::InstrumentWidget(parent), ui(new Ui::focusingwindow) {
	ui->setupUi(this);
	ui->imageWidget->setInfoVisible(false);

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

	// when the focusinghistory widget selects a position, we want
	// the focusercontroller to learn about it
	connect(ui->focusinghistoryWidget, SIGNAL(positionSelected(int)),
		ui->focusercontrollerWidget, SLOT(setTarget(int)));

	// wiring up the scan controller
	connect(ui->scanWidget, SIGNAL(movetoPosition(int)),
		ui->focusercontrollerWidget, SLOT(movetoPosition(int)));
	connect(ui->focusercontrollerWidget, SIGNAL(targetPositionReached()),
		ui->scanWidget, SLOT(positionReached()));
	connect(ui->scanWidget, SIGNAL(performCapture()),
		ui->ccdcontrollerWidget, SLOT(captureClicked()));
	connect(ui->ccdcontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		ui->scanWidget,
		SLOT(imageReceived(astro::image::ImagePtr)));
}

/**
 * \brief Destroy the focusing widget
 */
focusingwindow::~focusingwindow() {
	delete ui;
}

void	focusingwindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);
	ui->ccdcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->focusercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->guideportcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->adaptiveopticscontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->mountcontrollerWidget->instrumentSetup(serviceobject, instrument);

	setAppname("Focusing");
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
void	focusingwindow::receiveImage(ImagePtr _image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new %s image received",
		_image->size().toString().c_str());
	image(_image);

	// inform other widgets
	ui->imageWidget->setImage(_image);
	Exposure	imageexposure
		= ui->ccdcontrollerWidget->imageexposure();
	ui->ccdcontrollerWidget->setExposure(imageexposure);

	// add the point to the focuspointwidgeth
	int	pos = ui->focusercontrollerWidget->getCurrentPosition();
	ui->focusinghistoryWidget->add(_image, pos);

	// emit a signal for saving
	if (_image) {
		sendImage(_image, std::string("focusing image"));
	}
}

/**
 * \brief Slot to handle a new rectangle
 *
 * This slot installs the rectangle as a sub frame of the CCD controller
 */
void	focusingwindow::rectangleSelected(ImageRectangle rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new rectangle: %s",
		rectangle.toString().c_str());
	ui->ccdcontrollerWidget->setSubframe(rectangle);
}

/**
 * \brief Make sure object is destroyed when the window closes
 */
void	focusingwindow::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui
