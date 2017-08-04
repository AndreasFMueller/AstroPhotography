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

singletrackwidget::singletrackwidget(QWidget *parent) :
	QWidget(parent), ui(new Ui::singletrackwidget) {
	ui->setupUi(this);

	_masperpixel = 1000;

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
}

singletrackwidget::~singletrackwidget() {
    delete ui;
}

void	singletrackwidget::add(const snowstar::TrackingPoint& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point");
	snowstar::TrackingPoint	p = point;
	p.timeago = astro::Timer::gettime() - point.timeago;
	_points.push_back(p);
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
	double	scale = _masperpixel / 1000;
	datatype_t	dt = _datatype;
	std::for_each(_points.begin(), _points.end(),
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
			cdw->add(p.timeago, a);
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
