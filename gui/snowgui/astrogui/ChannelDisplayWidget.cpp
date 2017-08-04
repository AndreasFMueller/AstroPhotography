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
	double	notbefore = _notafter - width() / _timescale;
	// draw contents
	draw(notbefore, _notafter);
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
	double	M = std::max(_channels.allMax(width()),
			-_channels.allMin(width()));

	// ensure that the range is at list 3 pixels
	if (M <  1.5) { M =  1.5; }
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

	// draw the channels
	ChannelPainter	channelpainter(painter);
	channelpainter.notbefore(notbefore);
	channelpainter.notafter(notafter);
	channelpainter.yscale(yscale);
	channelpainter.width(width());
	channelpainter.height(height());

	// draw the time lines
	double	timestep = 60;
	double	ticdistance = timestep * width() / (notafter - notbefore);
	if (ticdistance < 50) {
		timestep *= 5;
		ticdistance *= 5;
	}
	if (ticdistance < 50) {
		timestep *= 2;
		ticdistance *= 2;
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

	QFont   labelfont;
        labelfont.setPointSize(10);
        painter.setFont(labelfont);

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

} // namespace snowgui

