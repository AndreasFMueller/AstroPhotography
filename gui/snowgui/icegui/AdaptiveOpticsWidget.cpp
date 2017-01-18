/*
 * AdaptiveOpticsWidget.cpp -- implementation of adaptive optics widget
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "AdaptiveOpticsWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <AstroDebug.h>
#include <cmath>

namespace snowgui {

AdaptiveOpticsWidget::AdaptiveOpticsWidget(QWidget *parent) : QLabel(parent) {
	_x = 0;
	_y = 0;
};

AdaptiveOpticsWidget::~AdaptiveOpticsWidget() {
}

bool	AdaptiveOpticsWidget::setPoint(QPointF p) {
	// ignore the point if it is outside the allowed circle
	if (hypot(p.x(), p.y()) > 100) {
		return false;
	}
	_x = p.x();
	_y = p.y();
	repaint();
	return true;
}

void	AdaptiveOpticsWidget::emitPoint(QPoint p) {
	QPointF	target(100. * (p.x() - width() / 2.) / _radius,
			-100. * (p.y() - height() / 2.) / _radius);
	if (setPoint(target)) {
		emit pointSelected(target);
	}
}

void	AdaptiveOpticsWidget::mousePressEvent(QMouseEvent * e) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mouse press %d/%d",
		e->pos().x(), e->pos().y());
	emitPoint(e->pos());
}

void	AdaptiveOpticsWidget::mouseMoveEvent(QMouseEvent *e) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mouse move: %d/%d",
		e->pos().x(), e->pos().y());
	emitPoint(e->pos());
}

void	AdaptiveOpticsWidget::mouseReleaseEvent(QMouseEvent * /* e */) {
}

void	AdaptiveOpticsWidget::paintEvent(QPaintEvent * /* event */) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "paint event");
	draw();
}

void	AdaptiveOpticsWidget::draw() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the adaptive optics state");
	// get a painter
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// draw the circle of allowed positions
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing the black circle");
	QPointF	center(width() / 2., height() / 2.);
	_radius = std::min(width(), height()) / 2.;
	QPainterPath	circle;
	circle.addEllipse(center, _radius, _radius);
	QColor	black(0, 0, 0);
	painter.fillPath(circle, black);

	// draw a red circle at the target position
	QPointF	target(center.x() + _radius * _x / 100.,
			center.y() - _radius * _y / 100.);
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "red circle at %.1f,%.1f",
	//	target.x(), target.y());
	QPainterPath	redcircle;
	redcircle.addEllipse(target, 5, 5);
	QColor	red(255, 0, 0);
	painter.fillPath(redcircle, red);
	QPainterPath	whitecenter;
	whitecenter.addEllipse(target, 2, 2);
	QColor	white(255, 255, 255);
	painter.fillPath(whitecenter, white);
}

} // namespace snowgui
