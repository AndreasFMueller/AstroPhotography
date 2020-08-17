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
#include <AstroUSB.h>
#include <QAction>
#include "DeviceAction.h"
#include <sstream>

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

	// make sure the focuser stuff is initally not visible
	ui->focuserGroup->setVisible(false);
	ui->exposureGroup->setVisible(false);

	// don't display the metadata portion of the imagedisplaywidget
	ui->imageWidget->crosshairs(true);
	ui->imageWidget->setInfoVisible(false);
	ui->imageWidget->setRectangleSelectionEnabled(true);

	// prevent starting until we have a CCD
	ui->startButton->setEnabled(false);
	ui->singleButton->setEnabled(false);

	_mode = idle;

	// get a list of cameras
	_ccdMenu = menuBar()->addMenu(QString("Cameras"));
	_focuserMenu = menuBar()->addMenu(QString("Focusers"));

	// set up the work classes
	_exposurework = NULL;
	_streamwork = NULL;

	// create a thread to collect cameras
	CameraLister	*_lister = new CameraLister(NULL);
	connect(_lister, SIGNAL(camera(std::string)),
		this, SLOT(addCamera(std::string)));
	connect(_lister, SIGNAL(focuser(std::string)),
		this, SLOT(addFocuser(std::string)));
	connect(_lister, SIGNAL(finished()),
		_lister, SLOT(deleteLater()));
	_lister->start();

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
	connect(ui->singleButton, SIGNAL(clicked()),
		this, SLOT(singleClicked()));
	connect(ui->focuserSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(focusChanged(int)));

	// initialize the exposure structure
	_exposure.exposuretime(1);

	// initialize the timer
	_timer.setInterval(100);
	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(focuserUpdate()));
	_timer.start();

	// set up the context menu for the focuser
	ui->focuserSpinBox->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->focuserSpinBox,
		SIGNAL(customContextMenuRequested(const QPoint &)),
		this,
		SLOT(showFocuserStepsMenu(const QPoint &)));
}

/**
 * \brief Destroy the liveview main window
 */
LiveView::~LiveView() {
	delete ui;
	delete _exposurework;
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
		Devices	_devices(ModuleRepository::get());
		_ccd = _devices.getCcd(cameraname);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open device %s: %s",
			cameraname.c_str(), x.what());
	}

	// return if we have no camera
	if (!_ccd) {
		return;
	}
	ui->exposureGroup->setVisible(true);

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

	// inform in the status bar that we have found a new camera
	statusBar()->showMessage(QString(astro::stringprintf("New camera: %s",
		cameraname.c_str()).c_str()));

	// XXX make sure the menu is updated or displayed at all
	_ccdMenu->raise();
}

/**
 * \brief Slot called when a focuser is selected
 *
 * \param focusername	name of the focuser to open
 */
void	LiveView::openFocuser(std::string focusername) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening focuser: %s",
		focusername.c_str());

	try {
		Devices	_devices(ModuleRepository::get());
		_focuser = _devices.getFocuser(focusername);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open device %s: %s",
			focusername.c_str(), x.what());
	}

	// return if we have no camera
	if (!_focuser) {
		return;
	}

	// configure the focuser group
	ui->focuserSpinBox->blockSignals(true);
	ui->focuserSpinBox->setMinimum(_focuser->min());
	ui->focuserSpinBox->setMaximum(_focuser->max());
	ui->focuserSpinBox->setValue(_focuser->current());
	ui->focuserSpinBox->blockSignals(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser maximum: %d",
		ui->focuserSpinBox->maximum());

	// make the focuser componentent visible
	ui->focuserGroup->setVisible(true);

	// window title
	updateTitle();
}

/**
 * \brief Create an informative title
 */
void	LiveView::updateTitle() {
	std::ostringstream	out;
	out << "LiveView";
	if (_ccd) {
		out << " @ " << _ccd->name();
	}
	if (_focuser) {
		out << " (focuser: " << _focuser->name().toString() << ")";
	}
	setWindowTitle(QString(out.str().c_str()));
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

	// inform in the status bar that we have found a new camera
	statusBar()->showMessage(QString(astro::stringprintf("New focuser: %s",
		focusername.c_str()).c_str()));

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

	// create streamwork
	_streamwork = new StreamWork(this);
	_streamwork->interval(ui->intervalSpinBox->value());
	connect(ui->intervalSpinBox, SIGNAL(valueChanged(double)),
		_streamwork, SLOT(interval(double)));

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
	if (_mode == streaming) {
		stopStream();
		ui->startButton->setText(QString("Stream"));
		return;
	}
	_mode = streaming;
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
	_mode = idle;
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
	if (single == _mode) {
		_thread->terminate();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure done");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "disable start buttons");
	ui->startButton->setEnabled(false);
	ui->singleButton->setEnabled(false);

	// make sure we remember that we are processing single images
	_mode = single;

	// set up the thread to do the work
	_thread = new QThread(NULL);
	connect(_thread, SIGNAL(finished()), this, SLOT(threadFinished()));
	//connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));

	// set up the work to be done
	_exposurework = new ExposureWork(this);
	_exposurework->moveToThread(_thread);
	
	// connect to the exposure work
	connect(this, SIGNAL(triggerExposure()),
		_exposurework, SLOT(doExposure()));
	_thread->start();

	// send the signal to the thread 
	emit triggerExposure();
}

/**
 * \brief Slot called when an image is received
 *
 * This slot is used to forward the image received in the separate thread
 * to the main thread where it can be displayed
 */
void	LiveView::receiveImage(astro::image::ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image received");
	// update the status bar
	std::string	m = astro::stringprintf("%s of pixel type %s",
		image->info().c_str(),
		astro::demangle_cstr(image->pixel_type()));
	statusBar()->showMessage(QString(m.c_str()));

	if (_mode == single) {
		// cleanup
		return;
	}
}

/**
 * \brief Slot called when the thread finishes
 */
void	LiveView::threadFinished() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Thread finished");
	// clean up the thread member
	_thread = NULL;
	_mode = idle;

	// reenable the buttons
	ui->startButton->setEnabled(true);
	ui->singleButton->setEnabled(true);
}

/**
 * \brief Slot called when the focus changes
 */
void	LiveView::focusChanged(int value) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focus changed to %d", value);
	if (_focuser) {
		_focuser->set(value);
	}
}

/**
 * \brief Slot called by the timer to udpate the focuser info
 */
void	LiveView::focuserUpdate() {
	if (!_focuser) {
		return;
	}
	long	c = _focuser->current();
	long	t = ui->focuserSpinBox->value();
	if (c != t) {
		statusBar()->showMessage(QString(
			astro::stringprintf("Focuser moving: %d of %d",
				c, t).c_str()
		));
	} else {
		statusBar()->showMessage(QString(
			astro::stringprintf("Focuser at %d", c).c_str()
		));
	}
}

/**
 * \brief Slot called when the context menu for the focuser is requested
 *
 * \param Point where the menu should be displayed
 */
void	LiveView::showFocuserStepsMenu(const QPoint& p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "show focuser context menu at %d,%d",
		p.x(), p.y());

	int	stepsize = ui->focuserSpinBox->singleStep();

	QMenu	contextMenu("Change step size");

	QAction	action1(QString("1"), this);
	action1.setCheckable(true);
	action1.setChecked(stepsize == 1);
	action1.setData(QVariant::fromValue((int)1));
	contextMenu.addAction(&action1);
	connect(&action1, SIGNAL(triggered()),
		this, SLOT(stepsizeChanged()));

	QAction	action10(QString("10"), this);
	action10.setCheckable(true);
	action10.setChecked(stepsize == 10);
	action10.setData(QVariant::fromValue((int)10));
	contextMenu.addAction(&action10);
	connect(&action10, SIGNAL(triggered()),
		this, SLOT(stepsizeChanged()));

	QAction	action100(QString("100"), this);
	action100.setCheckable(true);
	action100.setChecked(stepsize == 100);
	action100.setData(QVariant::fromValue((int)100));
	if (ui->focuserSpinBox->maximum() >= 100) {
		contextMenu.addAction(&action100);
		connect(&action100, SIGNAL(triggered()),
			this, SLOT(stepsizeChanged()));
	}

	QAction	action1000(QString("1000"), this);
	action1000.setCheckable(true);
	action1000.setChecked(stepsize == 1000);
	action1000.setData(QVariant::fromValue((int)1000));
	if (ui->focuserSpinBox->maximum() >= 1000) {
		contextMenu.addAction(&action1000);
		connect(&action1000, SIGNAL(triggered()),
			this, SLOT(stepsizeChanged()));
	}

	QAction	action10000(QString("10000"), this);
	action10000.setCheckable(true);
	action10000.setChecked(stepsize == 10000);
	action10000.setData(QVariant::fromValue((int)10000));
	if (ui->focuserSpinBox->maximum() >= 10000) {
		contextMenu.addAction(&action10000);
		connect(&action10000, SIGNAL(triggered()),
			this, SLOT(stepsizeChanged()));
	}

	QAction	action100000(QString("100000"), this);
	action100000.setCheckable(true);
	action100000.setChecked(stepsize == 100000);
	action100000.setData(QVariant::fromValue((int)100000));
	if (ui->focuserSpinBox->maximum() >= 100000) {
		contextMenu.addAction(&action100000);
		connect(&action100000, SIGNAL(triggered()),
			this, SLOT(stepsizeChanged()));
	}

	contextMenu.exec(ui->focuserSpinBox->mapToGlobal(p));
}

/**
 * \brief Slot called when the step size is changed
 */
void	LiveView::stepsizeChanged() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stepsize changed");
	astro::usb::USBdebugEnable();
	QAction *act = qobject_cast<QAction *>(sender());
	QVariant	v = act->data();
	int	stepsize = v.value<int>();
	ui->focuserSpinBox->setSingleStep(stepsize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stepsize changed: %d", stepsize);
}

} // namespace snowgui
