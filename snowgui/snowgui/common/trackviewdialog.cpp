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

trackviewdialog::trackviewdialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::trackviewdialog) {
	ui->setupUi(this);

	// create two channels in the widget
	ui->gpWidget->addChannel(QColor(0., 255., 0.));
	ui->gpWidget->addChannel(QColor(0., 0., 255.));

	// make sure the data type is initially set to offsetPx
	_datatype = offsetPx;

	// make sure the calibration structures are in a consistent state
	_gpcalibration.id = -1;
	_gpcalibration.guider.ccdIndex = -1;
	_gpcalibration.guider.guiderportIndex = -1;
	_gpcalibration.guider.adaptiveopticsIndex = -1;
	_aocalibration.id = -1;
	_aocalibration.guider.ccdIndex = -1;
	_aocalibration.guider.guiderportIndex = -1;
	_aocalibration.guider.adaptiveopticsIndex = -1;

	// make sure the track is clean
	_track.guiderunid = -1;
	_track.guiderportcalid = -1;
	_track.adaptiveopticscalid = -1;
	_track.guider.ccdIndex = -1;
	_track.guider.guiderportIndex = -1;
	_track.guider.adaptiveopticsIndex = -1;

	// connect only at the end
	connect(ui->offsetPxButton, SIGNAL(toggled(bool)),
		this, SLOT(buttonToggled(bool)));
	connect(ui->offsetArcsecButton, SIGNAL(toggled(bool)),
		this, SLOT(buttonToggled(bool)));
	connect(ui->correctionButton, SIGNAL(toggled(bool)),
		this, SLOT(buttonToggled(bool)));

}

trackviewdialog::~trackviewdialog() {
	delete ui;
}

void	trackviewdialog::setGuiderFactory(snowstar::GuiderFactoryPrx guiderfactory) {
	_guiderfactory = guiderfactory;
}

void	trackviewdialog::setTrack(snowstar::TrackingHistory track) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got new track: %d", track.guiderunid);
	if (track.guiderunid < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "bad track");
		return;
	}
	_track = track;

	// set the title
	std::string	title = astro::stringprintf("track: %d",
		track.guiderunid);
	setWindowTitle(QString(title.c_str()));

	// get the calibrations
	_gpcalibration.id = -1;
	try {
		if ((_guiderfactory) && (_track.guiderportcalid > 0)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve gp cal %d",
				_track.guiderportcalid);
			_gpcalibration = _guiderfactory->getCalibration(
						_track.guiderportcalid);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "gp cal %d", 
				_track.guiderportcalid);
		}
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "problem retrieving gp cal");
	}
	_aocalibration.id = -1;
	try {
		if ((_guiderfactory) && (_track.adaptiveopticscalid > 0)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve ao cal %d",
				_track.adaptiveopticscalid);
			_aocalibration = _guiderfactory->getCalibration(
						_track.adaptiveopticscalid);
		}
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "problem retrieving ao cal");
	}

	// make sure we only allow angle display if we have a calibration
	if (_gpcalibration.id > 0) {
		ui->offsetArcsecButton->setEnabled(true);
	} else {
		ui->offsetArcsecButton->setEnabled(false);
	}

	// update the data
	updateData();
}

void	trackviewdialog::updateData() {
	if (_track.guiderunid < 0) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating data track: %d",
		_track.guiderunid);
	// copy the data to the channels
	ChannelDisplayWidget	*cdw = ui->gpWidget;
	cdw->clearData();
	int	counter = 0;
	double	scale = 1;
	datatype_t	dt = _datatype;
	if ((_datatype == offsetArcsec) && (_gpcalibration.id > 0)) {
		scale = _gpcalibration.masPerPixel / 1000.;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "scale: %f", scale);
	}
	std::for_each(_track.points.begin(), _track.points.end(),
		[cdw,&counter,dt,scale](const snowstar::TrackingPoint& p) mutable {
			std::vector<double>	a;
			double	x, y;
			switch (dt) {
			case offsetPx:
				x = p.trackingoffset.x;
				y = p.trackingoffset.y;
				break;
			case offsetArcsec:
				x = p.trackingoffset.x * scale;
				y = p.trackingoffset.y * scale;
				break;
			case correction:
				x = p.activation.x;
				y = p.activation.y;
				break;
			}
			a.push_back(x);
			a.push_back(y);
			cdw->add(a);
			counter++;
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "channels: %d, points %d",
		ui->gpWidget->channels(), counter);

	ui->gpWidget->repaint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repaint complete");
}

void	trackviewdialog::buttonToggled(bool t) {
	if (!t) {
		return;
	}
	if (sender() == ui->offsetPxButton) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "data type changed to offsetPx");
		_datatype = offsetPx;
	}
	if (sender() == ui->offsetArcsecButton) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "data type changed to offsetArcsec");
		_datatype = offsetArcsec;
	}
	if (sender() == ui->correctionButton) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "data type changed to correction");
		_datatype = correction;
	}
	updateData();
}

} // namespace snowgui
