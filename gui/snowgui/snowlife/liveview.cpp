/*
 * liveview.cpp -- liveview main window implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"
#include "ui_liveview.h"
#include <AstroDebug.h>
#include <CameraLister.h>

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

	// don't display the metadata portion of the imagedisplaywidget
	ui->imageWidget->crosshairs(true);
	ui->imageWidget->setInfoVisible(false);

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
void	LiveView::openCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening camera");
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
	QAction	*action = new QAction(QString(cameraname.c_str()), this);
	connect(action, &QAction::triggered,
		this, &LiveView::openCamera);
	_ccdMenu->addAction(action);

	// XXX make sure the menu is updated or displayed at all
}

/**
 * \brief Slot called when a focuser is selected
 */
void	LiveView::openFocuser() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening focuser");
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
	QAction	*action = new QAction(QString(focusername.c_str()), this);
	connect(action, &QAction::triggered,
		this, &LiveView::openFocuser);
	_focuserMenu->addAction(action);

	// XXX make sure the menu is updated or displayed at all
}

} // namespace snowgui
