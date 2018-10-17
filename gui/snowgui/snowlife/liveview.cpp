/*
 * liveview.cpp -- liveview main window implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"
#include "ui_liveview.h"
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Construct the liveview main window
 *
 * \param parent	parent widget
 */
LiveView::LiveView(QWidget *parent)
	: QMainWindow(parent), ui(new Ui::LiveView) {
	ui->setupUi(this);

	// XXX fake a few camera names
	_ccdNames.push_back("camera:simulator/imager");
	_ccdNames.push_back("camera:simulator/finder");
	_ccdNames.push_back("camera:simulator/guider");
	_ccdNames.push_back("camera:asi/r8");

	// get a list of cameras
	_ccdMenu = menuBar()->addMenu(QString("Cameras"));
	for (auto i = _ccdNames.begin(); i != _ccdNames.end(); i++) {
		QAction	*action = new QAction(QString(i->c_str()), this);
		connect(action, &QAction::triggered,
			this, &LiveView::openCamera);
		_ccdMenu->addAction(action);
	}
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

} // namespace snowgui
