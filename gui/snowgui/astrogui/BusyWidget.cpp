/*
 * BusyWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "BusyWidget.h"
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <AstroDebug.h>
#include <cmath>

namespace snowgui {

/**
 * \brief Construct a transparent widget
 *
 * \param parent	parent widget
 */
BusyWidget::BusyWidget(QWidget *parent) : QWidget(parent) {
	_timer.setInterval(50);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer.start();

	// start the clock
	_start.start();
}

/**
 * \brief Destroy the widget
 */
BusyWidget::~BusyWidget() {
	_timer.stop();
}

/**
 * \brief draw the spinning wheel
 */
void	BusyWidget::draw() {
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// use the _start member to get the time
	double	_time = _start.gettime() - _start.startTime();

	// find the center and the dimensions
	QPointF	center(width() / 2., height() / 2.);
	float	l = ((width() > height()) ? width() : height()) / 2.;

	// fill the rectangle
	QPainterPath	rectangle;
	rectangle.moveTo(0,0);
	rectangle.lineTo(width(), 0);
	rectangle.lineTo(width(), height());
	rectangle.lineTo(0, height());
	rectangle.lineTo(0, 0);
	QColor	black(0, 0, 0);
	painter.fillPath(rectangle, black);

	// draw a circle
	QPainterPath	circle;
	circle.addEllipse(center, l, l);
	QColor	dark(0, 0, 0);
	painter.fillPath(circle, dark);

	// dimensions of the rotating stars
	l = 0.8 * l;
	float	s = 0.6 * l;
	const	int N = 7;
	const	float phistep = M_PI / N;

	// rotating star
	QPainterPath	star;
	float	phi = 30 * _time * M_PI / 180;
	QPointF	start(center.x() + l * cos(phi), center.y() - l * sin(phi));
	star.moveTo(start);
	for (int i = 1; i <= 2 * N;) {
		QPointF	low(center.x() + s * cos(phi + i * phistep),
			    center.y() - s * sin(phi + i * phistep));
		star.lineTo(low);
		i++;
		QPointF	high(center.x() + l * cos(phi + i * phistep),
			     center.y() - l * sin(phi + i * phistep));
		star.lineTo(high);
		i++;
	}
	QColor	gray(204, 204, 204);
	painter.fillPath(star, gray);
}

/**
 * \brief Event called when drawing is needed
 */
void	BusyWidget::paintEvent(QPaintEvent* /* event */) {
	draw();
}

/**
 * \brief Update slot called by the timer
 */
void	BusyWidget::update() {
	repaint();
}

/**
 * \brief start the busy indicator
 */
void	BusyWidget::start() {
	_timer.start();
	_start.start();
}

/**
 * \brief stop the busy indicator
 */
void	BusyWidget::stop() {
	_timer.stop();
	_start.end();
}

} // namespace snowgui
