/*
 * trackselectiondialog.cpp -- implementation of track selection dialog
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "trackselectiondialog.h"
#include "ui_trackselectiondialog.h"
#include <IceConversions.h>

namespace snowgui {

trackselectiondialog::trackselectiondialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::trackselectiondialog) {
	ui->setupUi(this);

	connect(this, SIGNAL(accepted()),
		this, SLOT(trackAccepted()));

	setWindowTitle("Select Track");
}

trackselectiondialog::~trackselectiondialog() {
	delete ui;
}

static std::string	formatlabel(const snowstar::TrackingHistory& track) {
	time_t	when = snowstar::converttime(track.timeago);
	struct tm	*tmp = localtime(&when);
	char	buffer[200];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	return astro::stringprintf("%03d: %s, %6d points", track.guiderunid,
		buffer, track.points.size());
}

void	trackselectiondialog::setGuider(
		snowstar::GuiderDescriptor guiderdescriptor,
		snowstar::GuiderFactoryPrx guiderfactory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set the track selection %s",
		guiderdescriptor.instrumentname.c_str());

	_guiderdescriptor = guiderdescriptor;
	_guiderfactory = guiderfactory;

	// update the title
	std::string	title = astro::stringprintf("Select Track %s",
		_guiderdescriptor.instrumentname.c_str());
	setWindowTitle(QString(title.c_str()));

	// empty the track list
	_tracks.clear();

	// Font
	QFont	font("Fixed");
	font.setStyleHint(QFont::Monospace);

	// read all tracks for that guider
	snowstar::idlist	trackids
		= _guiderfactory->getGuideruns(_guiderdescriptor);
	snowstar::idlist::const_iterator	i;
	for (i = trackids.begin(); i != trackids.end(); i++) {
		snowstar::TrackingHistory		track
			= _guiderfactory->getTrackingHistory(*i);
		_tracks.push_back(track);
		std::string	label = formatlabel(track);
		QString	ls(label.c_str());
		QListWidgetItem	*item = new QListWidgetItem(ls);
		item->setFont(font);
		ui->tracklistWidget->addItem(item);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "track selection initialized");
}

void	trackselectiondialog::trackAccepted() {
	int	selected = ui->tracklistWidget->currentRow();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "track %d selected, id %d, %d points",
		selected, _tracks[selected].guiderunid,
		_tracks[selected].points.size());
	snowstar::TrackingHistory	track = _tracks[selected];
	emit trackSelected(track);
}

} // namespace snowgui
