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

trackingmonitordialog::trackingmonitordialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::trackingmonitordialog) {
	ui->setupUi(this);
}

trackingmonitordialog::~trackingmonitordialog() {
	delete ui;
}

void	trackingmonitordialog::add(const snowstar::TrackingPoint& point) {
	switch (point.type) {
	case snowstar::ControlGuiderPort:
		ui->gpTrack->add(point);
		break;
	case snowstar::ControlAdaptiveOptics:
		ui->aoTrack->add(point);
		break;
	}
}

void	trackingmonitordialog::add(const snowstar::TrackingHistory& history) {
	std::string	title = astro::stringprintf("Track %d", history.trackid);
	setWindowTitle(QString(title.c_str()));
	trackingmonitordialog	*myself = this;

	int	counter = 0;
	std::for_each(history.points.begin(), history.points.end(),
		[myself,&counter](snowstar::TrackingPoint p) {
			myself->add(p);
			counter++;
		}
	);
	updateData();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added %d points", counter);
}

void	trackingmonitordialog::updateData() {
	ui->gpTrack->updateData();
	ui->aoTrack->updateData();
}

void	trackingmonitordialog::gpMasperpixel(double masperpixel) {
	ui->gpTrack->masperpixel(masperpixel);
}

void	trackingmonitordialog::aoMasperpixel(double masperpixel) {
	ui->gpTrack->masperpixel(masperpixel);
}

} // namespace snowgui
