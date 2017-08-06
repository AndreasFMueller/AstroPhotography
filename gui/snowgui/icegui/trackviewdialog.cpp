/*
 * trackviewdialog.cpp -- dialog to view a track
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "trackviewdialog.h"
#include "ui_trackviewdialog.h"
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace snowgui {

/**
 * \brief Construct a trackview dialog
 */
trackviewdialog::trackviewdialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::trackviewdialog) {
	ui->setupUi(this);
}

/**
 * \brief Destroy a trackview dialog
 */
trackviewdialog::~trackviewdialog() {
	delete ui;
}

/**
 * \brief Give the trackviewdialog a guider factory proxy
 */
void	trackviewdialog::setGuiderFactory(snowstar::GuiderFactoryPrx guiderfactory) {
	_guiderfactory = guiderfactory;
}

/**
 * \brief Select a track
 *
 * This method gets the complete track history to display
 */
void	trackviewdialog::setTrack(snowstar::TrackingHistory track) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got new track: %d", track.trackid);
	if (track.trackid < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad track");
		return;
	}
	_track = track;

	// set the title
	std::string	title = astro::stringprintf("track: %d",
		track.trackid);
	setWindowTitle(QString(title.c_str()));

	// update the data
	ui->trackWidget->updateData();
}

} // namespace snowgui
