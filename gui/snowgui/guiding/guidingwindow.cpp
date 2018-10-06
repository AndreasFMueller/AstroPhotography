/*
 * guidingwindow.cpp -- implementation of guiding window
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "guidingwindow.h"
#include "ui_guidingwindow.h"
#include <AstroImage.h>

using namespace astro;

namespace snowgui {

/**
 * \brief Constructor for the guiding window
 */
guidingwindow::guidingwindow(QWidget *parent) : InstrumentWidget(parent),
	ui(new Ui::guidingwindow) {
	ui->setupUi(this);
	// set up settings that cannot be controlled by the designed
	ui->imageWidget->setInfoVisible(false);
	ui->imageWidget->setRectangleSelectionEnabled(false);
	ui->imageWidget->setPointSelectionEnabled(true);
	ui->imagercontrollerWidget->hideSubframe(true);

	// when a point is selected in the image widget, then the guider
	// controller should set it as the star, and it should create 
	// a small window around it
	connect(ui->imageWidget,
		SIGNAL(pointSelected(astro::image::ImagePoint)),
		ui->guidercontrollerWidget,
		SLOT(selectPoint(astro::image::ImagePoint)));

	// when the ccd controller receives an image, the image widget should
	// display it
	connect(ui->imagercontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		ui->imageWidget,
		SLOT(receiveImage(astro::image::ImagePtr)));
	connect(ui->imagercontrollerWidget,
		SIGNAL(exposureChanged(astro::camera::Exposure)),
		ui->guidercontrollerWidget,
		SLOT(setExposure(astro::camera::Exposure)));
	connect(ui->imagercontrollerWidget,
		SIGNAL(imageReceived(astro::image::ImagePtr)),
		this, SLOT(newImage(astro::image::ImagePtr)));
}

guidingwindow::~guidingwindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy guidingwindow");
	delete ui;
}

/**
 * \brief Instrument setup
 *
 * Propagate instrument information to all the components that need it
 */
void	guidingwindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	ui->imagercontrollerWidget->launchInstrumentSetup(serviceobject,
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
	ui->guidercontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
	ui->mountcontrollerWidget->launchInstrumentSetup(serviceobject,
		instrument);
}

void	guidingwindow::setupComplete() {
	setAppname("Guiding");
}

/**
 * \brief New image received
 */
void	guidingwindow::newImage(astro::image::ImagePtr _image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image received, offer for saving");
	sendImage(_image, std::string("guiding"));
}

/**
 * \brief handle window close event
 *
 * This event handler makes sure the window is destroyed when it is closed
 */
void	guidingwindow::closeEvent(QCloseEvent * /* event */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "allow deletion");
	sendImage(astro::image::ImagePtr(NULL), std::string());
	deleteLater();
}

} // namespace snowgui
