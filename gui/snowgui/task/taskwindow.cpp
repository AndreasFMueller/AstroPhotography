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
		this,
		SLOT(receiveImage(astro::image::ImagePtr)));
	connect(ui->taskqueuemanagerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		this,
		SLOT(receiveImage(astro::image::ImagePtr)));
	connect(this,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		ui->imagedisplayWidget,
		SLOT(receiveImage(astro::image::ImagePtr)));

	connect(ui->imagedisplayWidget,
		SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
		ui->ccdcontrollerWidget,
		SLOT(setSubframe(astro::image::ImageRectangle)));

	// setup connections between devices and the submission
	connect(ui->filterwheelcontrollerWidget,
		SIGNAL(filterwheelSelected(snowstar::FilterWheelPrx)),
		ui->tasksubmissionWidget,
		SLOT(filterwheelSelected(snowstar::FilterWheelPrx)));
	connect(ui->ccdcontrollerWidget,
		SIGNAL(exposureChanged(astro::camera::Exposure)),
		ui->tasksubmissionWidget,
		SLOT(exposureChanged(astro::camera::Exposure)));
	connect(ui->ccdcontrollerWidget,
		SIGNAL(ccdSelected(int)),
		ui->tasksubmissionWidget,
		SLOT(ccdSelected(int)));
	connect(ui->coolercontrollerWidget,
		SIGNAL(coolerSelected(int)),
		ui->tasksubmissionWidget,
		SLOT(coolerSelected(int)));
	connect(ui->filterwheelcontrollerWidget,
		SIGNAL(filterwheelSelected(int)),
		ui->tasksubmissionWidget,
		SLOT(filterwheelSelected(int)));
	connect(ui->mountcontrollerWidget,
		SIGNAL(mountSelected(int)),
		ui->tasksubmissionWidget,
		SLOT(mountSelected(int)));

	// set up connections with this class
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
	ui->tasksubmissionWidget->instrumentSetup(serviceobject, instrument);
	ui->taskmonitorWidget->setServiceObject(serviceobject);
	ui->taskqueuemanagerWidget->setServiceObject(serviceobject);
	setAppname("Tasks");
}

/**
 * \brief Handle closing of this window
 */
void	taskwindow::closeEvent(QCloseEvent * /* event */) {
	sendImage(ImagePtr(NULL), std::string());
	deleteLater();
}

/**
 * \brief Slot to handle an image that was received from the CCD
 */
void	taskwindow::receiveImage(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received an image %s",
		image->size().toString().c_str());
	ui->feedbackWidget->setCurrentIndex(1);
	emit imageReceived(image);
	sendImage(image, std::string("task"));
}

/**
 * \brief Slot to handle selection of a rectangle in the image
 */
void	taskwindow::rectangleSelected(ImageRectangle rectangle) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle %s selected",
		rectangle.toString().c_str());
}

} // namespace snowgui
