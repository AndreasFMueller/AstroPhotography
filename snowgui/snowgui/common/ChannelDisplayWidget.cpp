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
	std::deque<double>	newchannel;
	_channels.push_back(newchannel);
	_colors.push_back(color);
}

/**
 * \brief Add a new point
 */
void	ChannelDisplayWidget::add(std::vector<double> values) {
	int	nvalues = values.size();
	if (nvalues != channels()) {
		std::string	msg = astro::stringprintf("wrong number of "
			"values: %d != %d", values.size(), channels());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	for (int i = 0; i < nvalues; i++) {
		_channels[i].push_back(values[i]);
	}
}

/**
 * \brief Handle the paintEvent
 *
 * This simply calls the 
 */
void	ChannelDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Perform the drawing itself
 */
void	ChannelDisplayWidget::draw() {
	// draw the white background
	QPainter	painter(this);
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
	double	M = std::max(allMax(), -allMin());

	// ensure that the range is at list 3 pixels
	if (M <  1.5) { M =  1.5; }

	// compute the scale in such a way that the maximum value is at least
	// one pixel away from the border. With this value of the scale,
	// y coordinates are coputed as y * yscale + height() / 2
	double	yscale = (height() - 2) / (2 * M);

	// compute standard deviations and means
	std::vector<double>	mean;
	std::vector<double>	stddev;
	for (int i = 0; i < channels(); i++) {
		const std::deque<double>&	channel = _channels[i];
		std::deque<double>::const_reverse_iterator	r;
		r = channel.crbegin();
		double	sum = 0;
		double	sum2 = 0;
		int	w = 1;
		do {
			double	y = *r;
			sum += y;
			sum2 += y * y;
			r++;
			w++;
		} while ((w <= width()) && (r != channel.crend()));
		double	m = sum / w;
		mean.push_back(m);
		double	s = sqrt((w / (w - 1.)) * (sum2 / w) - m * m);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"channel %d: mean = %.3f, stddev = %.3f", i, m, s);
		stddev.push_back(s);
	}

	// construct color rectangles
	ColorRectangles	rectangles;
	for (size_t i = 0; i < mean.size(); i++) {
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
		const std::deque<double>&	channel = _channels[i];
		std::deque<double>::const_reverse_iterator	r;
		r = channel.crbegin();
		int	w = 1;
		double	y = *r;
		QPoint	p(width() - w, height() / 2 - 1 - yscale * y);
		r++; w++;
		do {
			y = *r;
			QPoint	q(width() - w, height() / 2 - 1 - yscale * y);
			painter.drawLine(p, q);
			p = q;
			r++; w++;
		} while ((w <= width()) && (r != channel.crend()));
		// compute std deviation
	}
}

/**
 * \brief Find the minimum value of all channels
 */
double	ChannelDisplayWidget::allMin() {
	std::vector<double>	minima;
	for (int i = 0; i < channels(); i++) {
		minima.push_back(channelMin(i));
	}
	return *min_element(minima.begin(), minima.end());
}

/**
 * \brief Find the maximum value of all channels
 */
double	ChannelDisplayWidget::allMax() {
	std::vector<double>	maxima;
	for (int i = 0; i < channels(); i++) {
		maxima.push_back(channelMax(i));
	}
	return *min_element(maxima.begin(), maxima.end());
}

/**
 * \brief Find the minimum value a a given channel
 */
double	ChannelDisplayWidget::channelMin(int channelid) {
	int	l = _channels[channelid].size();
	std::deque<double>::const_reverse_iterator	b
		= _channels[channelid].rbegin();
	std::deque<double>::const_reverse_iterator	e;
	if (l >= width()) {
		e = b + width() - 1;
	} else {
		e = _channels[channelid].rend();
	}
	return *min_element(b, e);
}

/**
 * \brief Find the maximum value a a given channel
 */
double	ChannelDisplayWidget::channelMax(int channelid) {
	int	l = _channels[channelid].size();
	std::deque<double>::const_reverse_iterator	b
		= _channels[channelid].rbegin();
	std::deque<double>::const_reverse_iterator	e;
	if (l >= width()) {
		e = b + width() - 1;
	} else {
		e = _channels[channelid].rend();
	}
	return *max_element(b, e);
}

/**
 * \brief Clear the data
 */
void	ChannelDisplayWidget::clearData() {
	std::for_each(_channels.begin(), _channels.end(),
		[](std::deque<double>& channel) mutable {
			channel.clear();
		}
	);
}

} // namespace snowgui

