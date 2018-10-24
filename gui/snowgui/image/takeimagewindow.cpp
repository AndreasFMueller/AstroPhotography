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
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <IceConversions.h>
#include <camera.h>

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

	// make sure we get the CCD proxy from the ccdcontroller
	connect(ui->ccdcontrollerWidget,
		SIGNAL(ccdprxSelected(snowstar::CcdPrx)),
		this, SLOT(setCcd(snowstar::CcdPrx)));

	// connect the startStream signal from the ccdcontrollerwidget
	connect(ui->ccdcontrollerWidget, SIGNAL(streamStart()),
		this, SLOT(setupStream()));
	connect(this, SIGNAL(startStream()),
		ui->ccdcontrollerWidget, SLOT(startStream()));
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
void	takeimagewindow::setupComplete() {
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

/**
 * \brief Set up the stream
 */
void	takeimagewindow::setupStream() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setupStream()");
	if (_ccd) {
		debug(LOG_ERR, DEBUG_LOG, 0, "now CCD");
	}

	// create the Image Sink
	debug(LOG_DEBUG, DEBUG_LOG, 0, "should start the stream now");

	// create the image sink
	try {
		// debug create the image sink
		_imagesink = new TakeImageSink(this);

		// connect the sink to the communicator
		snowstar::CommunicatorSingleton::connect(_ccd);
		Ice::ObjectPtr  _sinkptr(_imagesink);
		_sinkidentity = snowstar::CommunicatorSingleton::add(_sinkptr);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "identity: %s",
			_sinkidentity.name.c_str());

		// register the image sink
		_ccd->registerSink(_sinkidentity);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot create image sink: %s",
			x.what());
	}

	// connect the sink to the image display widget
	connect(_imagesink, SIGNAL(newImage(astro::image::ImagePtr)),
		ui->imageWidget, SLOT(receiveImage(astro::image::ImagePtr)));
	connect(_imagesink, SIGNAL(finished()),
		this, SLOT(streamFinished()));

	// emit the signal to tell the ccdcontrollerwidget that the
	// stream should now be started
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the stream");
	emit startStream();
}

/**
 * \brief set the proxy
 */
void	takeimagewindow::setCcd(snowstar::CcdPrx ccd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd proxy");
	_ccd = ccd;
}

/**
 * \brief Slot to tell the window that the stream has finished
 */
void	takeimagewindow::streamFinished() {
	_ccd->unregisterSink();
}

} // namespace snowgui
