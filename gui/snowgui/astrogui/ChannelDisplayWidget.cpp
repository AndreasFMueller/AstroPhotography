/*
 * ChannelDisplayWidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ChannelDisplayWidget.h>
#include <QPainter>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include "ColorRectangles.h"

namespace snowgui {

/**
 * \brief Construct a new channel display widget
 */
ChannelDisplayWidget::ChannelDisplayWidget(QWidget *parent) : QWidget(parent) {
	_timescale = 1;
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
}

/**
 * \brief Handle the paintEvent
 *
 * This simply calls the 
 */
void	ChannelDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	double	notafter = _channels.allLast();
	double	notbefore = notafter - width() / _timescale;
	draw(notbefore, notafter);
}

/**
 * \brief Perform the drawing itself
 */
void	ChannelDisplayWidget::draw(double notbefore, double notafter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"plotting between %.1f and %.1f (%.1f seconds)",
		notbefore, notafter, notafter - notbefore);
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
	double	M = std::max(_channels.allMax(width()), -_channels.allMin(width()));

	// ensure that the range is at list 3 pixels
	if (M <  1.5) { M =  1.5; }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "M = %f", M);

	// compute the scale in such a way that the maximum value is at least
	// one pixel away from the border. With this value of the scale,
	// y coordinates are coputed as y * yscale + height() / 2
	double	yscale = (height() - 2) / (2 * M);

	// compute standard deviations and means
	std::vector<double>	mean = _channels.mean(width());
	std::vector<double>	stddev = _channels.stddev(width());

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangles drawn");

	// prepare a pen
	QPen	pen(Qt::SolidLine);
	pen.setWidth(1);

	// draw zero line of the coordinate system
	pen.setColor(QColor(0., 0., 0.));
	painter.setPen(pen);
	double	y = height() / 2.;
	painter.drawLine(QPointF(0, y), QPointF(width()-1, y));

	// draw the various level lines
	pen.setColor(QColor(180., 180., 180.));
	painter.setPen(pen);
	int	m = 1;
	while ((m * yscale) < (height() / 2)) {
		double	y = m * yscale + height() / 2.;
		painter.drawLine(QPointF(0, y), QPointF(width() - 1, y));
		y = -m * yscale + height() / 2.;
		painter.drawLine(QPointF(0, y), QPointF(width() - 1, y));
		m++;
	}

	// draw channels
	for (int i = 0; i < channels(); i++) {
		pen.setColor(_colors[i]);
		painter.setPen(pen);
		const ChannelData&	channel = _channels[i];
		ChannelData::const_reverse_iterator	r;
		r = channel.crbegin();
		int	w = 1;
		double	y = r->value;
		QPoint	p(width() - w, height() / 2 - 1 - yscale * y);
		r++; w++;
		do {
			y = r->value;
			QPoint	q(width() - w, height() / 2 - 1 - yscale * y);
			painter.drawLine(p, q);
			p = q;
			r++; w++;
		} while ((w <= width()) && (r != channel.crend()));
	}
}

/**
 * \brief Clear the data
 */
void	ChannelDisplayWidget::clearData() {
	_channels.clear();
}

} // namespace snowgui

