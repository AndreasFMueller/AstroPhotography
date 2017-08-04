/*
 * ChannelDisplayWidget.cpp -- implementation of ChannelDisplayWidget
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ChannelDisplayWidget.h>
#include <QPainter>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include "ColorRectangles.h"
#include <QScrollArea>
#include <QScrollBar>

namespace snowgui {

/**
 * \brief Construct a new channel display widget
 */
ChannelDisplayWidget::ChannelDisplayWidget(QWidget *parent) : QWidget(parent) {
	_timescale = 1;
	_vscale = 1;
}

/**
 * \brief Destroy the channel display
 */
ChannelDisplayWidget::~ChannelDisplayWidget() {
}

/**
 * \brief Retrieve the number of channels to display
 */
int	ChannelDisplayWidget::channels() const {
	return _channels.size();
}

/**
 * \brief Add information for a new channel
 *
 * Make sure you call repaint after this event so that the display gets updated
 */
void	ChannelDisplayWidget::addChannel(QColor color) {
	ChannelData	newchannel;
	_channels.push_back(newchannel);
	_colors.push_back(color);
}

/**
 * \brief Add a new point
 */
void	ChannelDisplayWidget::add(double time, std::vector<double> values) {
	int	nvalues = values.size();
	if (nvalues != channels()) {
		std::string	msg = astro::stringprintf("wrong number of "
			"values: %d != %d", values.size(), channels());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	for (int i = 0; i < nvalues; i++) {
		ChannelDataPoint	datapoint(time, values[i]);
		_channels[i].push_back(datapoint);
	}
	// compute new width
	double	duration = _channels.allLast() - _channels.allFirst();
	int	newwidth = duration / _timescale;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "newwidth = %d", newwidth);
	this->setMinimumSize(newwidth, 0);
}

/**
 * \brief Handle the paintEvent
 *
 * This simply calls the 
 */
void	ChannelDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	if (autorange()) {
		_notafter = _channels.allLast();
	}
	double	notbefore = _channels.allFirst();
	// draw contents
	draw(notbefore, _notafter);
}

/**
 * \brief Perform the drawing itself
 */
void	ChannelDisplayWidget::draw(double notbefore, double notafter) {
	double	duration = notafter - notbefore;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"plotting between %.1f and %.1f (%.1f seconds)",
		notbefore, notafter, duration);

	int	newwidth = duration / _timescale; // [timescale] = [s/pixel]
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"timescale = %f, newwidth = %d, width = %f",
		_timescale, newwidth, (double)width());
	this->setMinimumSize(newwidth, 0);

	// draw the white background
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));

	// first check that we have enough data to reasonably draw something
	if (channels() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no channels to draw");
		return;
	}
	int	l = _channels[0].size();
	if (l < 2) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not enough data to draw");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing %d channels with %d points",
		channels(), l);

	// find the maximum and minimum of all channels
	double	M = std::max(_channels.allMax(), -_channels.allMin());

	// ensure that the range is at least 3 pixels
	if (M < 1.5) { M = 1.5; }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "M = %f", M);

	// compute the scale in such a way that the maximum value is at least
	// one pixel away from the border. With this value of the scale,
	// y coordinates are coputed as y * yscale + height() / 2
	double	yscale = _vscale * (height() - 2) / (2 * M);

	// compute standard deviations and means
	std::vector<double>	mean = _channels.mean(notbefore, notafter);
	std::vector<double>	stddev = _channels.stddev(notbefore, notafter);

	// construct color rectangles
	ColorRectangles	rectangles;
	for (size_t i = 0; i < mean.size(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mean[%d] = %f, stddev[%d] = %f",
			i, mean[i], i, stddev[i]);
		double	h  = height() / 2 - 1;
		double	bottom = h - (mean[i] - stddev[i]) * yscale;
		double	top = h - (mean[i] + stddev[i]) * yscale;
		Color	color = Color(_colors[i]) * 0.1;
		rectangles.addRange(top, bottom, color);
	}
	rectangles.draw(painter, width());

	// prepare a pen
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);

	// draw zero line of the coordinate system
	pen.setColor(QColor(0., 0., 0.));
	painter.setPen(pen);
	double	y = height() / 2.;
	painter.drawLine(QPointF(0, y), QPointF(width()-1, y));

	// set the font size
	QFont   labelfont;
        labelfont.setPointSize(10);
        painter.setFont(labelfont);

	// draw the various level lines
	int	deltam = 1;
	while ((yscale * deltam * _vscale) < 15) {
		deltam *= 10;
	}
	int	m = deltam;
	while ((m * yscale) < (height() / 2)) {
		double	y = m * yscale + height() / 2.;
		pen.setColor(QColor(180., 180., 180.));
		painter.setPen(pen);
		painter.drawLine(QPointF(0, y), QPointF(width() - 1, y));
		pen.setColor(QColor(0., 0., 0.));
		painter.setPen(pen);
		double	x = width() - 20;
		while (x > 0) {
			painter.drawText(x - 40, y - 6, 40, 12,
				Qt::AlignRight,
				QString(astro::stringprintf("%d", -m).c_str()));
			x -= 300;
		}

		y = -m * yscale + height() / 2.;
		pen.setColor(QColor(180., 180., 180.));
		painter.setPen(pen);
		painter.drawLine(QPointF(0, y), QPointF(width() - 1, y));
		pen.setColor(QColor(0., 0., 0.));
		painter.setPen(pen);
		x = width() - 20;
		while (x > 0) {
			painter.drawText(x - 40, y - 6, 40, 12,
				Qt::AlignRight,
				QString(astro::stringprintf("%d", m).c_str()));
			x -= 300;
		}

		m += deltam;
	}

	// draw the channels
	ChannelPainter	channelpainter(painter);
	channelpainter.notbefore(notbefore);
	channelpainter.notafter(notafter);
	channelpainter.yscale(yscale);
	channelpainter.width(newwidth);
	channelpainter.height(height());

	// draw the time lines
	pen.setColor(QColor(180., 180., 180.));
	painter.setPen(pen);
	double	timestep = 60;
	double	ticdistance = timestep / _timescale; // [_timescale] = [s/pixel]
	while (ticdistance < 50) {
		timestep *= 10;
		ticdistance *= 10;
	}
	double	t = timestep * floor(notafter / timestep);
	while (t > notbefore) {
		double	x = channelpainter.X(t);
		painter.drawLine(QPoint(x, 0), QPoint(x, height()));
		t -= timestep;
	}

	channelpainter(_channels, _colors);

	pen.setColor(QColor(0., 0., 0.));
	painter.setPen(pen);

	// draw the time labels
	t = timestep * floor(notafter / timestep);
	while (t > notbefore) {
		// format the time
		time_t	tt = t;
		struct tm	*tp = localtime(&tt);
		std::string	timelabel = astro::stringprintf("%02d:%02d",
					tp->tm_hour, tp->tm_min);
		
		// draw the time label
		QPoint	p(channelpainter.X(t), 0);
		painter.drawText(p.x() - 20, p.y(), 40, 15,
			Qt::AlignCenter, QString(timelabel.c_str()));
		t -= timestep;
	}
}

/**
 * \brief Clear the data
 */
void	ChannelDisplayWidget::clearData() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "clearing data");
	_channels.clear();
}

/**
 * \brief change the scale 
 */
void	ChannelDisplayWidget::setScale(int v) {
	setVscale(v);
	repaint();
}

/**
 * \brief change the time scale
 */
void	ChannelDisplayWidget::setTime(int t) {
	// [timescale] = [s/pixel]
	setTimescale(pow(2, -t));
	repaint();
}

} // namespace snowgui

