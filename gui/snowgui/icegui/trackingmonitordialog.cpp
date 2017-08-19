/*
 * trackingmonitordialog.cpp -- implementation of tracking monitor
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "trackingmonitordialog.h"
#include "ui_trackingmonitordialog.h"
#include <AstroDebug.h>
#include <AstroFormat.h>

namespace snowgui {

/**
 * \brief Construct a new trackmonitordialog widget
 */
trackingmonitordialog::trackingmonitordialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::trackingmonitordialog) {
	ui->setupUi(this);
	ui->gpWidget->setVisible(false);
	ui->aoWidget->setVisible(false);
}

/**
 * \brief Destroy the trackingmonitordialog
 */
trackingmonitordialog::~trackingmonitordialog() {
	delete ui;
}

/**
 * \brief add a point to the tracks
 */
void	trackingmonitordialog::add(const snowstar::TrackingPoint& point) {
	switch (point.type) {
	case snowstar::ControlGuidePort:
		ui->gpTrack->add(point);
		break;
	case snowstar::ControlAdaptiveOptics:
		ui->aoTrack->add(point);
		break;
	}
}

/**
 * \brief add a tracking history
 */
void	trackingmonitordialog::add(const snowstar::TrackingHistory& history) {
	std::string	title = astro::stringprintf("Track %d", history.trackid);
	setWindowTitle(QString(title.c_str()));
	trackingmonitordialog	*myself = this;
	clearData();

	int	counter = 0;
	std::for_each(history.points.begin(), history.points.end(),
		[myself,&counter](snowstar::TrackingPoint p) mutable {
			myself->add(p);
			counter++;
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d points", counter);
	ui->gpWidget->setVisible(ui->gpTrack->hasData());
	ui->aoWidget->setVisible(ui->aoTrack->hasData());
}

/**
 * \brief Update the data
 */
void	trackingmonitordialog::updateData() {
	ui->gpTrack->updateData();
	ui->aoTrack->updateData();
}

/**
 * \brief Clear the data
 */
void	trackingmonitordialog::clearData() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "clear requested");
	ui->gpTrack->clearData();
	ui->aoTrack->clearData();
}

/**
 * \brief set the mas per pixel scale for the guide port
 */
void	trackingmonitordialog::gpMasperpixel(double masperpixel) {
	ui->gpTrack->masperpixel(masperpixel);
}

/**
 * \brief set the mas per pixel scale for the adaptive optics port
 */
void	trackingmonitordialog::aoMasperpixel(double masperpixel) {
	ui->gpTrack->masperpixel(masperpixel);
}

/**
 * \brief Set the mas per pixel scale from a calibration
 */
void    trackingmonitordialog::calibration(const snowstar::Calibration& calibration) {
	switch (calibration.type) {
	case snowstar::ControlGuidePort:
		gpMasperpixel(calibration.masPerPixel);
		break;
	case snowstar::ControlAdaptiveOptics:
		aoMasperpixel(calibration.masPerPixel);
		break;
	}
}

void	trackingmonitordialog::refreshDisplay() {
	ui->gpTrack->refreshDisplay();
	ui->gpWidget->setVisible(ui->gpTrack->hasData());
	ui->aoTrack->refreshDisplay();
	ui->aoWidget->setVisible(ui->aoTrack->hasData());
}

} // namespace snowgui
