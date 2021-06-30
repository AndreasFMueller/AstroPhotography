/*
 * trackselectiondialog.cpp -- implementation of track selection dialog
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "trackselectiondialog.h"
#include "ui_trackselectiondialog.h"
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Construct a trackselection dialog
 */
trackselectiondialog::trackselectiondialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::trackselectiondialog) {
	ui->setupUi(this);
	

	connect(this, SIGNAL(accepted()),
		this, SLOT(trackAccepted()));

	setWindowTitle("Select Track");
}

/**
 * \brief Destroy the track selection dialog
 */
trackselectiondialog::~trackselectiondialog() {
	delete ui;
}

/**
 * \brief Auxiliary function to format the label for a track
 */
static std::string	formatlabel(const snowstar::TrackingSummary& track) {
	time_t	when = snowstar::converttime(track.since);
	struct tm	*tmp = localtime(&when);
	char	buffer[200];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	return astro::stringprintf("%03d: %s, %d points", track.trackid,
		buffer, track.points);
}

/**
 * \brief Set the guider
 *
 * This triggers the dialog to retrieve the list of tracks for this
 * particular guider.
 */
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

	// empty the track list and also the contents of the tracklistWidget
	_tracks.clear();
	ui->tracklistWidget->blockSignals(true);
	while (ui->tracklistWidget->count() > 0) {
		QListWidgetItem	*lwi = ui->tracklistWidget->item(0);
                ui->tracklistWidget->removeItemWidget(lwi);
        }

	// Font
	QFont	font("Fixed");
	font.setStyleHint(QFont::Monospace);

	// read all tracks for that guider
	snowstar::idlist	trackids
		= _guiderfactory->getTracks(_guiderdescriptor);
	snowstar::idlist::const_iterator	i;
	for (i = trackids.begin(); i != trackids.end(); i++) {
		snowstar::TrackingSummary		track
			= _guiderfactory->getTrackingSummary(*i);
		_tracks.push_back(track);
		std::string	label = formatlabel(track);
		QString	ls(label.c_str());
		QListWidgetItem	*item = new QListWidgetItem(ls);
		item->setFont(font);
		ui->tracklistWidget->addItem(item);
	}
	ui->tracklistWidget->blockSignals(false);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "track selection initialized");
}

/**
 * \brief Slot called when a track is selected
 *
 * This slot retrieves the complete tracking history (it used a summary
 * only to fill the selection list) and emits the trackSelected signal
 * with the complete track history as the argument.
 */
void	trackselectiondialog::trackAccepted() {
	int	selected = ui->tracklistWidget->currentRow();
	if ((selected < 0) || (selected >= (int)_tracks.size())) {
		return;
	}
	int	trackid = _tracks[selected].trackid;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "selected track id: %d", trackid);
	snowstar::TrackingHistory	track
		= _guiderfactory->getTrackingHistory(trackid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "track %d selected, id %d, %d points",
		selected, _tracks[selected].trackid, track.points.size());
	emit trackSelected(track);
}

} // namespace snowgui
