/*
 * GuiderButton.cpp 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderButton.h>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <cmath>
#include <AstroDebug.h>

namespace snowgui {

GuiderButton::GuiderButton(QWidget *parent) : QWidget(parent) {
	_northPressed = false;
	_southPressed = false;
	_westPressed = false;
	_eastPressed = false;
}

GuiderButton::~GuiderButton() {
}

void	GuiderButton::draw() {
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// common parameters
	QColor	background(0, 0, 0);
	painter.fillRect(0, 0, width(), height(), background);
	QPointF	center(width() / 2., height() / 2.);
	QColor	grey(224, 224, 224);
	QColor	red(255, 128, 128);
	
	// ellipse
	double	w = width() - 10;
	double	h = height() - 10;
	QRect	rect(5, 5, w, h);
	QPainterPath	ellipse;

	ellipse.moveTo(center);
	ellipse.arcTo(rect, 45, 90);
	ellipse.closeSubpath();
	painter.fillPath(ellipse, (_northPressed) ? red : grey);

	ellipse = QPainterPath();
	ellipse.moveTo(center);
	ellipse.arcTo(rect, 135, 90);
	ellipse.closeSubpath();
	painter.fillPath(ellipse, (_eastPressed) ? red : grey);

	ellipse = QPainterPath();
	ellipse.moveTo(center);
	ellipse.arcTo(rect, 225, 90);
	ellipse.closeSubpath();
	painter.fillPath(ellipse, (_southPressed) ? red : grey);

	ellipse = QPainterPath();
	ellipse.moveTo(center);
	ellipse.arcTo(rect, 315, 90);
	ellipse.closeSubpath();
	painter.fillPath(ellipse, (_westPressed) ? red : grey);

	// black lines 
	QPainterPath	bezel;
	bezel.addRect(0, 0, width(), height());
	bezel.arcTo(rect, 0, 360);
	QColor	black(0, 0, 0);
	painter.fillPath(bezel, black);

	QPainterPath	bar;
	bar.moveTo(0, 5);
	bar.lineTo(width() - 5, height());
	bar.lineTo(width(), height() - 5);
	bar.lineTo(5, 0);
	bar.closeSubpath();
	painter.fillPath(bar, black);

	bar = QPainterPath();
	bar.moveTo(width() - 5, 0);
	bar.lineTo(0, height() - 5);
	bar.lineTo(5, height());
	bar.lineTo(width(), 5);
	bar.closeSubpath();
	painter.fillPath(bar, black);
}

void	GuiderButton::paintEvent(QPaintEvent * /* event */) {
	draw();
}

void	GuiderButton::mousePressEvent(QMouseEvent *e) {
	checkPressed(e->pos());
	repaint();
}

void	GuiderButton::mouseMoveEvent(QMouseEvent *e) {
	checkPressed(e->pos());
	repaint();
}

void	GuiderButton::mouseReleaseEvent(QMouseEvent *e) {
	checkPressed(e->pos());
	if (_northPressed) {
		emit northClicked();
		_northPressed = false;
	}
	if (_southPressed) {
		emit southClicked();
		_southPressed = false;
	}
	if (_westPressed) {
		emit westClicked();
		_westPressed = false;
	}
	if (_eastPressed) {
		emit eastClicked();
		_eastPressed = false;
	}
	repaint();
}

double	GuiderButton::angle(QPoint p) {
	double	w = width() / 2.;
	double	h = height() / 2.;
	double	x = (p.x() - w) / w;
	double	y = (p.y() - h) / h;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f, y = %f", x, y);
	return atan2(y, x);
}

void	GuiderButton::checkPressed(QPoint p) {
	double	a = angle(p);
	if (a < 0) {
		a += 2 * M_PI;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "angle = %f", 180 * a / M_PI);
	if ((a > (M_PI / 4)) && ((3 * M_PI / 4) >= a)) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "South");
		_northPressed = false;
		_westPressed = false;
		_southPressed = true;
		_eastPressed = false;
	} else if ((a > (3 * M_PI / 4)) && ((5 * M_PI / 4) > a)) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "West");
		_northPressed = false;
		_westPressed = false;
		_southPressed = false;
		_eastPressed = true;
	} else if ((a > (5 * M_PI / 4)) && ((7 * M_PI / 4) > a)) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "North");
		_northPressed = true;
		_westPressed = false;
		_southPressed = false;
		_eastPressed = false;
	} else {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "East");
		_northPressed = false;
		_westPressed = true;
		_southPressed = false;
		_eastPressed = false;
	}
}

} // namespace snowgui
