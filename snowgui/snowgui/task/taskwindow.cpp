/*
 * taskwindow.cpp -- implementation of the task window
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "taskwindow.h"
#include "ui_taskwindow.h"
#include <imagedisplaywidget.h>

namespace snowgui {

/**
 * \brief Create a new task window
 */
taskwindow::taskwindow(QWidget *parent)
	: snowgui::InstrumentWidget(parent), ui(new Ui::taskwindow) {
	ui->setupUi(this);

	// configure the child widgets
	ui->imagedisplayWidget->setSubframeVisible(false);
	ui->imagedisplayWidget->setInfoVisible(false);
	ui->imagedisplayWidget->setRectangleSelectionEnabled(true);

	// set up connections between ccdcontroller and image display
	connect(ui->ccdcontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		ui->imagedisplayWidget,
		SLOT(receiveImage(astro::image::ImagePtr)));
	connect(ui->imagedisplayWidget,
		SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
		ui->ccdcontrollerWidget,
		SLOT(setSubframe(astro::image::ImageRectangle)));

	// set up connections with this class
	connect(ui->ccdcontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		this,
		SLOT(imageReceived(astro::image::ImagePtr)));
}

/**
 * \brief Destroy the task window
 */
taskwindow::~taskwindow() {
	delete ui;
}

/**
 * \brief configure instrument information
 *
 * This method also sets up all the widget children with the information
 * regarding the instrument
 */
void	taskwindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up instrument");

	InstrumentWidget::instrumentSetup(serviceobject, instrument);
	ui->ccdcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->taskstatusWidget->setServiceObject(serviceobject);
	setAppname("Tasks");
}

/**
 * \brief Handle closing of this window
 */
void	taskwindow::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

/**
 * \brief Slot to handle an image that was received from the CCD
 */
void	taskwindow::receiveImage(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received an image %s",
		image->size().toString().c_str());
}

/**
 * \brief Slot to handle selection of a rectangle in the image
 */
void	taskwindow::rectangleSelected(ImageRectangle rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle %s selected",
		rectangle.toString().c_str());
}

} // namespace snowgui
