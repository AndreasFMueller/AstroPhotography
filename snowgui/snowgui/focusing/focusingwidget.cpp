/*
 * focusingwidget.cpp -- implementation of the focusing widget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "focusingwidget.h"
#include "ui_focusingwidget.h"
#include <AstroImage.h>
#include <AstroCamera.h>
#include <imagedisplaywidget.h>
#include <FocusPoints.h>

using namespace astro::image;
using namespace astro::camera;

focusingwidget::focusingwidget(QWidget *parent)
	: snowgui::InstrumentWidget(parent), ui(new Ui::focusingwidget) {
	ui->setupUi(this);
	ui->imageWidget->setInfoVisible(false);
}

focusingwidget::~focusingwidget() {
	delete ui;
}

void	focusingwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);
	ui->ccdcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->focusercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject, instrument);

	connect(ui->imageWidget,
		SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
		this,
		SLOT(rectangleSelected(astro::image::ImageRectangle)));

	connect(ui->focusinghistoryWidget,
		SIGNAL(positionSelected(int)),
		ui->focusercontrollerWidget,
		SLOT(setTarget(int)));

	// wiring up the scan controller
	connect(ui->scanWidget, SIGNAL(movetoPosition(int)),
		ui->focusercontrollerWidget, SLOT(movetoPosition(int)));
	connect(ui->focusercontrollerWidget, SIGNAL(targetPositionReached()),
		ui->scanWidget, SLOT(positionReached()));
	connect(ui->scanWidget, SIGNAL(performCapture()),
		ui->ccdcontrollerWidget, SLOT(captureClicked()));
	connect(ui->ccdcontrollerWidget, SIGNAL(imageReceived()),
		ui->scanWidget, SLOT(imageReceived()));
}

void	focusingwidget::imageReceived() {
	ImagePtr	image = ui->ccdcontrollerWidget->image();
	ui->imageWidget->setImage(image);
	Exposure	imageexposure = ui->ccdcontrollerWidget->imageexposure();
	ui->ccdcontrollerWidget->setExposure(imageexposure);

	// add the point to the focuspointwidgeth
	int	pos = ui->focusercontrollerWidget->getCurrentPosition();
	ui->focusinghistoryWidget->add(image, pos);

	// display focuspoint information
	snowgui::FocusPoint	fp(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus point: %s",
		fp.toString().c_str());
}

void	focusingwidget::rectangleSelected(ImageRectangle rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new rectangle: %s",
		rectangle.toString().c_str());
	ui->ccdcontrollerWidget->setSubframe(rectangle);
}
