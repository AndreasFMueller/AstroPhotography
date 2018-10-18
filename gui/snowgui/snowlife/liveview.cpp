/*
 * liveview.cpp -- liveview main window implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"
#include "ui_liveview.h"
#include <AstroDebug.h>
#include <CameraLister.h>
#include <AstroLoader.h>
#include <QAction>
#include "DeviceAction.h"

using namespace astro::device;
using namespace astro::module;

namespace snowgui {

/**
 * \brief Construct the liveview main window
 *
 * \param parent	parent widget
 */
LiveView::LiveView(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::LiveView) {
	ui->setupUi(this);

	qRegisterMetaType<std::string>("std::string");
	qRegisterMetaType<astro::image::ImagePtr>("astro::image::ImagePtr");
	qRegisterMetaType<astro::image::ImageRectangle>(
		"astro::image::ImageRectangle");

	// don't display the metadata portion of the imagedisplaywidget
	ui->imageWidget->crosshairs(true);
	ui->imageWidget->setInfoVisible(false);
	ui->imageWidget->setRectangleSelectionEnabled(true);

	// prevent starting until we have a CCD
	ui->startButton->setEnabled(false);
	ui->singleButton->setEnabled(false);

	// get a list of cameras
	_ccdMenu = menuBar()->addMenu(QString("Cameras"));
	_focuserMenu = menuBar()->addMenu(QString("Focusers"));

	// set up the work classes
	_work = new ExposureWork(this);

	_streamwork = new StreamWork(this);
	_streamwork->interval(ui->intervalSpinBox->value());

	// create a thread to collect cameras
	CameraLister	*_lister = new CameraLister(NULL);
	connect(_lister, SIGNAL(camera(std::string)),
		this, SLOT(addCamera(std::string)));
	connect(_lister, SIGNAL(focuser(std::string)),
		this, SLOT(addFocuser(std::string)));
	connect(_lister, SIGNAL(finished()),
		_lister, SLOT(deleteLater()));

	// connect the sink to the imageWidget
	connect(this,
		SIGNAL(newImage(astro::image::ImagePtr)),
		ui->imageWidget,
		SLOT(receiveImage(astro::image::ImagePtr)));
	connect(this,
		SIGNAL(newImage(astro::image::ImagePtr)),
		this,
		SLOT(receiveImage(astro::image::ImagePtr)));

	// connect buttons
	connect(ui->startButton, SIGNAL(clicked()),
		this, SLOT(startStream()));
	connect(ui->imageWidget,
		SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
		this,
		SLOT(setSubframe(astro::image::ImageRectangle)));
	connect(ui->fullframeButton, SIGNAL(clicked()),
		this, SLOT(fullframeClicked()));
	connect(ui->exposureSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(setExposuretime(double)));
	connect(ui->intervalSpinBox, SIGNAL(valueChanged(double)),
		_streamwork, SLOT(interval(double)));
	connect(ui->singleButton, SIGNAL(clicked()),
		this, SLOT(singleClicked()));

	// initialize the exposure structure
	_exposure.exposuretime(1);

	// start the lister thread
	_lister->start();
}

/**
 * \brief Destroy the liveview main window
 */
LiveView::~LiveView() {
	delete ui;
	delete _work;
	delete _streamwork;
}


/**
 * \brief Open camera menu action
 *
 * \param cameraname	name of the camera
 */
void	LiveView::openCamera(std::string cameraname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening camera: %s",
		cameraname.c_str());
	std::string	title = astro::stringprintf("LiveView %s",
				cameraname.c_str());
	setWindowTitle(QString(title.c_str()));

	// get the camera
	try {
		Devices	_devices(getModuleRepository());
		_ccd = _devices.getCcd(cameraname);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open device %s: %s",
			cameraname.c_str(), x.what());
	}

	// return if we have no camera
	if (!_ccd) {
		return;
	}

	// initialize the frame size of the exposure structure
	setSubframe(_ccd->getInfo().getFrame());
	ui->exposureSpinBox->setMinimum(_ccd->getInfo().minexposuretime());
	ui->exposureSpinBox->setMaximum(_ccd->getInfo().maxexposuretime());

	// enable start/stop
	ui->startButton->setEnabled(true);
	ui->singleButton->setEnabled(true);
}

/**
 * \brief Slot called when a new camera is added
 *
 * \param name of the camera
 */
void	LiveView::addCamera(std::string cameraname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new camera %s", cameraname.c_str());
	// remember the name
	_ccdNames.push_back(cameraname);

	// add a menu item for this camera
	DeviceAction	*action = new DeviceAction(cameraname,
				QString(cameraname.c_str()), this);
	connect(action, SIGNAL(openDevice(std::string)),
		this, SLOT(openCamera(std::string)));
	_ccdMenu->addAction(action);

	// XXX make sure the menu is updated or displayed at all
}

/**
 * \brief Slot called when a focuser is selected
 *
 * \param focusername	name of the focuser to open
 */
void	LiveView::openFocuser(std::string focusername) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening focuser: %s",
		focusername.c_str());
}

/**
 * \brief Slot called when a new focuser is detected
 *
 * \param focusername	name of the focuser
 */
void	LiveView::addFocuser(std::string focusername) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new focuser %s", focusername.c_str());
	// remember the name
	_ccdNames.push_back(focusername);

	// add a menu item for this focuser
	DeviceAction	*action = new DeviceAction(focusername,
				QString(focusername.c_str()), this);
	connect(action, SIGNAL(openDevice(std::string)),
		this, SLOT(openFocuser(std::string)));
	_focuserMenu->addAction(action);

	// XXX make sure the menu is updated or displayed at all
}

/**
 * \brief Process a new image
 *
 * \param entry		queue entry to process
 */
void	LiveView::operator()(const astro::camera::ImageQueueEntry& entry) {
	emit newImage(entry.image);
}

/**
 * \brief start private stream
 */
void	LiveView::startStreamPrivate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	if (_thread) {
		debug(LOG_ERR, DEBUG_LOG, 0, "there already is a thread");
	}

	// set up the thread to do the work
	_thread = new QThread;
	//connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
	connect(_thread, SIGNAL(finished()), this, SLOT(threadFinished()));
	_thread->start();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread created");

	// set up the work to be done
	_streamwork->moveToThread(_thread);
	QMetaObject::invokeMethod(_streamwork, "start", Qt::QueuedConnection);

	// change button status
	ui->singleButton->setEnabled(false);
	ui->startButton->setText(QString("Stop"));
}

/**
 * \brief Method to a privately managed stream
 */
void	LiveView::stopStreamPrivate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping private stream");
	_streamwork->stop();
	if (_thread) {
		_thread->terminate();
	}
}

/**
 * \brief Slot to start the stream
 */
void	LiveView::startStream() {
	if (!_ccd) {
		return;
	}
	if (_ccd->streaming() || _streamwork->running()) {
		stopStream();
		ui->startButton->setText(QString("Start"));
		return;
	}
	if (ui->intervalSpinBox->value() > 0) {
		startStreamPrivate();
		return;
	}
	ui->startButton->setText(QString("Stop"));
	_ccd->imagesink(this);
	_ccd->startStream(_exposure);
}

/**
 * \brief Slot to stop the stream
 */
void	LiveView::stopStream() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop stream");
	if (_streamwork) {
		stopStreamPrivate();
		return;
	}
	_ccd->stopStream();
}

/**
 * \brief Slot to set the subframe to display
 *
 * \param frame		the frame to use
 */
void	LiveView::setSubframe(astro::image::ImageRectangle frame) {
	_exposure.frame(frame);
	ui->rectangleField->setText(QString(frame.toString().c_str()));
}

/**
 * \brief Slot to revert to the full frame
 */
void	LiveView::fullframeClicked() {
	setSubframe(_ccd->getInfo().getFrame());
}

/**
 * \brief Slot to set the exposure time
 *
 * \param t	the exposure time to use
 */
void	LiveView::setExposuretime(double t) {
	_exposure.exposuretime(t);
}

/**
 * \brief Perform an exposure
 *
 * This method is called from the ExposureWork class and does the actual
 * exposing. This allows the ExposureWork class not to have any important
 * logic of it's own.
 */
void	LiveView::doExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start an exposure (t=%.3f)",
		_exposure.exposuretime());
	_ccd->startExposure(_exposure);
	_ccd->wait();
	astro::image::ImagePtr	image = _ccd->getImage();
	emit newImage(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure done");
}

/**
 * \brief Slot to perform a single exposure
 */
void	LiveView::doSingleExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "perform a single exposure");
	doExposure();
	_thread->terminate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread stopped");
}

/**
 * \brief Slot called when the single image button is clicked
 */
void	LiveView::singleClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "single clicked");
	if (!_ccd) {
		return;
	}

	// make sure no other action can be initiated while we are exposing
	ui->startButton->setEnabled(false);
	ui->singleButton->setEnabled(false);

	// make sure we remember that we are processing single images
	_single = true;

	// set up the thread to do the work
	_thread = new QThread(NULL);
	connect(_thread, SIGNAL(finished()), this, SLOT(threadFinished()));
	//connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
	connect(this, SIGNAL(doExposure()), _thread, SLOT(doSingleExposure()));
	_thread->start();

	// set up the work to be done
	_work = new ExposureWork(this);
	_work->moveToThread(_thread);
	
	// send the signal to the thread 
	emit doExposure();
}

/**
 * \brief Slot called when an image is received
 *
 * This slot is used to forward the image received in the separate thread
 * to the main thread where it can be displayed
 */
void	LiveView::receiveImage(astro::image::ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image received");
	if (!_single) {
		return;
	}

	// we are done processing single images
	_single = false;
}

/**
 * \brief Slot called when the thread finishes
 */
void	LiveView::threadFinished() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Thread finished");
	// clean up the thread member
	_thread = NULL;

	// reenable the buttons
	ui->startButton->setEnabled(true);
	ui->singleButton->setEnabled(true);
}

} // namespace snowgui
