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

	// get a list of cameras
	_ccdMenu = menuBar()->addMenu(QString("Cameras"));
	_focuserMenu = menuBar()->addMenu(QString("Focusers"));

	// create a thread to collect camears
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
}


/**
 * \brief Open camera menu action
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
 * \brief Slot to start the stream
 */
void	LiveView::startStream() {
	if (!_ccd) {
		return;
	}
	if (_ccd->streaming()) {
		stopStream();
		ui->startButton->setText(QString("Start"));
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
	_ccd->stopStream();
}

void	LiveView::setSubframe(astro::image::ImageRectangle frame) {
	_exposure.frame(frame);
	ui->rectangleField->setText(QString(frame.toString().c_str()));
}

void	LiveView::fullframeClicked() {
	setSubframe(_ccd->getInfo().getFrame());
}

void	LiveView::setExposuretime(double t) {
	_exposure.exposuretime(t);
}

} // namespace snowgui
