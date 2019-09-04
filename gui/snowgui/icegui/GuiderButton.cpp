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

static QString	northLabel("DEC+");
static QString	southLabel("DEC-");
static QString	westLabel("RA+");
static QString	eastLabel("RA-");

GuiderButton::GuiderButton(QWidget *parent) : QWidget(parent) {
	_northPressed = false;
	_southPressed = false;
	_westPressed = false;
	_eastPressed = false;

	_northActive = false;
	_southActive = false;
	_eastActive = false;
	_westActive = false;
}

GuiderButton::~GuiderButton() {
}

#define LEDDIM	6

void	GuiderButton::draw() {
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// common parameters
	QColor	background(0, 0, 0);
	painter.fillRect(0, 0, width(), height(), background);
	QPointF	center(width() / 2., height() / 2.);
	QColor	grey(224, 224, 224);
	QColor	red(255, 128, 128);
	QColor	brightred(255, 0, 0);
	
	// ellipse
	double	w = width() - 2 * LEDDIM;
	double	h = height() - 2 * LEDDIM;
	QRect	rect(LEDDIM, LEDDIM, w, h);
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
	bar.moveTo(0, LEDDIM);
	bar.lineTo(width() - LEDDIM, height());
	bar.lineTo(width(), height() - LEDDIM);
	bar.lineTo(LEDDIM, 0);
	bar.closeSubpath();
	painter.fillPath(bar, black);

	bar = QPainterPath();
	bar.moveTo(width() - LEDDIM, 0);
	bar.lineTo(0, height() - LEDDIM);
	bar.lineTo(LEDDIM, height());
	bar.lineTo(width(), LEDDIM);
	bar.closeSubpath();
	painter.fillPath(bar, black);

	// draw labels (RA+, RA-, DEC+, DEC-)
	painter.drawText(center.x() - w/2 + 5, center.y() - 8, 40, 20,
                Qt::AlignLeft, eastLabel);
	painter.drawText(center.x() + w/2 - 45, center.y() - 8, 40, 20,
                Qt::AlignRight, westLabel);
	painter.drawText(center.x() - 20, center.y() - h/2 + 00, 40, 20,
                Qt::AlignCenter, northLabel);
	painter.drawText(center.x() - 20, center.y() + h/2 - 20, 40, 20,
                Qt::AlignCenter, southLabel);

	// display the active LEDs
	if (_northActive) {
		QPainterPath	led;
		QPoint	ledcenter(width() / 2, LEDDIM/2);
		led.addEllipse(ledcenter, LEDDIM/2, LEDDIM/2);
		painter.fillPath(led, brightred);
	}
	if (_southActive) {
		QPainterPath	led;
		QPoint	ledcenter(width() / 2, height() - LEDDIM/2);
		led.addEllipse(ledcenter, LEDDIM/2, LEDDIM/2);
		painter.fillPath(led, brightred);
	}
	if (_eastActive) {
		QPainterPath	led;
		QPoint	ledcenter(3, height() / 2);
		led.addEllipse(ledcenter, LEDDIM/2, LEDDIM/2);
		painter.fillPath(led, brightred);
	}
	if (_westActive) {
		QPainterPath	led;
		QPoint	ledcenter(width() - LEDDIM/2, height() / 2);
		led.addEllipse(ledcenter, LEDDIM/2, LEDDIM/2);
		painter.fillPath(led, brightred);
	}
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
