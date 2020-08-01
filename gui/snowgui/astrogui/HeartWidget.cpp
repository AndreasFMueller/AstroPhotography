/*
 * HeartWidget.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <HeartWidget.h>
#include <QPainter>
#include <QPainterPath>

namespace snowgui {

/**
 * \brief Construct the heart widget
 *
 * \param parent	the parent widget
 */
HeartWidget::HeartWidget(QWidget *parent) : QWidget(parent) {
	_color = QColor(127, 127, 127);
	_timer.setInterval(100);
	_interval = 1;
	connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
}

/**
 * \brief Destroy the heart widget
 */
HeartWidget::~HeartWidget() {
	_timer.stop();
	_color = QColor(255, 127, 127);
}

/**
 * \brief Initiate a heartbeat update
 */
void	HeartWidget::beat() {
	_beattime = _start.gettime();
	_timer.start();
}

/**
 * \brief Update the heart image
 */
void	HeartWidget::update() {
	double	t = _start.gettime() - _beattime;
	double	l = exp(-2 * t / interval());
	int	r = 127;
	if (t > _interval) {
		_timer.stop();
	} else {
		r = (1 - l) * 127 + l * 255;
	}
	_color = QColor(r, 127, 127);
	repaint();
}

/**
 * \brief The paintEvent handler triggered by the repaint() method
 */
void	HeartWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Draw the heart in the current color
 */
void	HeartWidget::draw() {
	QPainter	painter(this);

	// determine dimensions of the heart
	double	h = height() / 6.;
	double	w = width() / 8.;
	double	s = (h < w) ? h : w;

	// compute some interesting points
	QPointF	center(width() / 2., height() / 2.);
	QPointF	A(center.x(), center.y() - s);
	QPointF	B(center.x(), center.y() + 3 * s);

	double	alpha = atan(0.5);
	double	r = 4 * s;

	QPointF	C(B.x() + r * sin(2 * alpha), B.y() - r * cos(2 * alpha));
	QPointF	D(B.x() - r * sin(2 * alpha), C.y());

	QRectF	rightlobe(center.x(), center.y() - 3 * s, 4 * s, 4 * s);
	QRectF	leftlobe(center.x() - 4 * s, center.y() - 3 * s, 4 * s, 4 * s);

	// construct the heart path
	QPainterPath	heartshape;
	heartshape.moveTo(B);
	heartshape.lineTo(C);
	double	degreealpha = 180 * 2 * alpha / M_PI;
	heartshape.arcTo(rightlobe, -degreealpha, 180);
	heartshape.arcTo(leftlobe, 0, 180 + degreealpha);
	heartshape.lineTo(B);
	heartshape.closeSubpath();

	// fill the heart with current color
	painter.fillPath(heartshape, _color);
}

} // namespace snowgui
