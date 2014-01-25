/*
 * calibrationpointwidget.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <calibrationpointwidget.h>
#include <AstroDebug.h>
#include <QPainter>
#include <cmath>

CalibrationPointWidget::CalibrationPointWidget(QWidget *parent)
	: QWidget(parent) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a calibration point widget");
}

CalibrationPointWidget::~CalibrationPointWidget() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point widget destroyed");
}

void	CalibrationPointWidget::addPoint(const point_t& point) {
	points.push_back(point);
}

void	CalibrationPointWidget::addPoint(const Astro::Point& point) {
	addPoint(point_t(point.x, point.y));
}

void	CalibrationPointWidget::clear() {
	points.clear();
}

void    CalibrationPointWidget::paintEvent(QPaintEvent *event) {
        debug(LOG_DEBUG, DEBUG_LOG, 0, "paint event");
        drawPoints();
}

void	CalibrationPointWidget::drawPoints() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing %d points", points.size());
	QPainter	painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));

	// found out what scale we are supposed to use
	point_t	center = points.front();
	points_t::const_iterator	p;
	double	xscale = 0.1;
	double	yscale = 0.1;
	for (p = points.begin(); p != points.end(); p++) {
		double	x = fabs(p->first - center.first);
		if (x > xscale) { xscale = x; }
		double	y = fabs(p->second - center.second);
		if (y > yscale) { yscale = y; }
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale: %f, %f", xscale, yscale);
	xscale = (width() - 4) / (2 * xscale);
	yscale = (height() - 4)  / (2 * yscale);
	double	scale = (xscale > yscale) ? yscale : xscale;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "common scale: %f", scale);

	// draw the circle
	if (circle) {
		if (scale > (height() / 2)) {
			scale = height() / 2;
		}
		if (scale > (width() / 2)) {
			scale = width() / 2;
		}
		painter.drawEllipse(width() / 2 - scale, height() / 2 - scale,
			2 * scale, 2 * scale);
	}

	// draw the grid
	painter.fillRect(width() / 2, 0, 1, height(), QColor(128., 128., 128.));
	painter.fillRect(0, height() / 2, width(), 1, QColor(128., 128., 128.));

	// display all the points
	for (p = points.begin(); p != points.end(); p++) {
		int	x = width() / 2 + scale * (p->first - center.first);
		int	y = height() / 2 - scale * (p->second - center.second);
		//painter.drawEllipse(x, y, 2, 2);
		painter.fillRect(x - 1, y - 1, 3, 3, color);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "redraw complete");
}
