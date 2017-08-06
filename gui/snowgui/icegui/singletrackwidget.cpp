/*
 * singletrackwidget.cpp -- display a single track
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "singletrackwidget.h"
#include "ui_singletrackwidget.h"
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace snowgui {

/**
 * \brief create a new singletrackwidget
 */
singletrackwidget::singletrackwidget(QWidget *parent) :
	QWidget(parent), ui(new Ui::singletrackwidget) {
	ui->setupUi(this);

	_masperpixel = 0;
	ui->offsetArcsecButton->setEnabled(false);

	// two channels
	ui->dataWidget->addChannel(QColor(0, 255, 0));
	ui->dataWidget->addChannel(QColor(0, 0, 255));

	// starting with px offset display
	_datatype = offsetPx;

	// connect only at the end
	connect(ui->offsetPxButton, SIGNAL(toggled(bool)),
		this, SLOT(buttonToggled(bool)));
	connect(ui->offsetArcsecButton, SIGNAL(toggled(bool)),
		this, SLOT(buttonToggled(bool)));
	connect(ui->correctionButton, SIGNAL(toggled(bool)),
		this, SLOT(buttonToggled(bool)));
	connect(ui->scaleDial, SIGNAL(valueChanged(int)),
		ui->dataWidget, SLOT(setScale(int)));
	connect(ui->timeDial, SIGNAL(valueChanged(int)),
		ui->dataWidget, SLOT(setTime(int)));
}

/**
 * \brief Destroy the widget
 */
singletrackwidget::~singletrackwidget() {
    delete ui;
}

/**
 * \brief set angular scale
 */
void	singletrackwidget::masperpixel(double m) {
	_masperpixel = m;
	ui->offsetArcsecButton->setEnabled(m > 0);
}

/**
 * \brief Get the arcsec scale from the calibration
 */
void	singletrackwidget::calibration(
		const snowstar::Calibration& calibration) {
	masperpixel(calibration.masPerPixel);
}

/**
 * \brief convert a point to the form used by the ChannelDisplayWidget
 *
 * \param point		tracking point to convert
 */
std::vector<double>	singletrackwidget::convert(
				const snowstar::TrackingPoint& point) const {
	std::vector<double>	a;
	double	x, y;
	switch (_datatype) {
	case offsetPx:
		x = point.trackingoffset.x;
		y = point.trackingoffset.y;
		break;
	case offsetArcsec:
		x = point.trackingoffset.x * _masperpixel / 1000;
		y = point.trackingoffset.y * _masperpixel / 1000;
		break;
	case correction:
		x = point.activation.x;
		y = point.activation.y;
		break;
	}
	a.push_back(x);
	a.push_back(y);
	return a;
}

/**
 * \brief add a new TrackingPoint
 */
void	singletrackwidget::add(const snowstar::TrackingPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point");
	snowstar::TrackingPoint	p = point;
	p.timeago = astro::Timer::gettime() - point.timeago;
	_points.push_back(p);

	// add the point in the right format to the 
	ui->dataWidget->add(p.timeago, convert(point));
}

/**
 * \brief add a track of a certain type
 */
void	singletrackwidget::add(const snowstar::TrackingHistory& track,
		const snowstar::ControlType type) {
	clearData();
	singletrackwidget	*stw = this;
	std::for_each(track.points.begin(), track.points.end(),
		[stw,type](const snowstar::TrackingPoint& point) mutable {
			if (point.type == type) {
				stw->add(point);
			}
		}
	);
	updateData();
}

/**
 * \brief Method called when the data changes
 *
 * This needs to be called e.g. when one switches from showing the offset
 * in pixels to arc seconds, or showing the correction instead of the
 * offset.
 */
void	singletrackwidget::updateData() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d points", _points.size());
	// copy the data to the channels
	ChannelDisplayWidget	*cdw = ui->dataWidget;
	cdw->clearData();
	int	counter = 0;
	singletrackwidget	*stw = this;
	std::for_each(_points.begin(), _points.end(),
		[cdw,&counter,stw](const snowstar::TrackingPoint& p) mutable {
			cdw->add(p.timeago, stw->convert(p));
			counter++;
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "channels: %d, points %d",
		cdw->channels(), counter);

	cdw->repaint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repaint complete");
}

/**
 * \brief Slot called when a button is toggled
 *
 * The buttons select the type of data that is displayed. When that changes,
 * the updateData() method needs to be called.
 */
void	singletrackwidget::buttonToggled(bool t) {
	if (!t) {
		return;
	}
	if (sender() == ui->offsetPxButton) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "data type changed to offsetPx");
		if (_datatype != offsetPx) {
			_datatype = offsetPx;
			updateData();
		}
	}
	if (sender() == ui->offsetArcsecButton) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"data type changed to offsetArcsec");
		if (_datatype != offsetArcsec) {
			_datatype = offsetArcsec;
			updateData();
		}
	}
	if (sender() == ui->correctionButton) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"data type changed to correction");
		if (_datatype != correction) {
			_datatype = correction;
			updateData();
		}
	}
}

/**
 * \brief clear the data
 */
void	singletrackwidget::clearData() {
	_points.clear();
	ui->dataWidget->clearData();
}

void	singletrackwidget::refreshDisplay() {
	ui->dataWidget->repaint();
}

} // namespace snowgui
