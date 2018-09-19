/*
 * CalibrationDisplayWidget.cpp -- implementation of the calibration display
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CalibrationDisplayWidget.h>
#include <QPainter>
#include <QEvent>
#include <AstroDebug.h>
#include <cmath>
#include "ColorArithmetic.h"

namespace snowgui {

/**
 * \brief Construct a calibration display widget
 */
CalibrationDisplayWidget::CalibrationDisplayWidget(QWidget *parent)
	: QWidget(parent) {
	_calibration.id = -1;
	_calibration.complete = false;
	_pointlabels = false;
}

/**
 * \brief Destroy the calibration display widget
 */
CalibrationDisplayWidget::~CalibrationDisplayWidget() {
}

/**
 * \brief Set the calibration
 */
void	CalibrationDisplayWidget::setCalibration(snowstar::Calibration calibration) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new calibration: %d, %d points",
		calibration.id, calibration.points.size());
	_calibration = calibration;
	repaint();
}

/**
 * \brief paint event, draw the calibration points and vectors
 */
void	CalibrationDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Main draw method
 *
 * This method calls drawDisabled and drawEnabled depending on the current
 * enable state of the widget.
 */
void	CalibrationDisplayWidget::draw() {
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	if (this->isEnabled()) {
		drawEnabled(painter);
	} else {
		drawDisabled(painter);
	}
}

/**
 * \brief Draw disabled state of the widget
 *
 * In the disabled state, vectors are never drawn, and points are drawn only
 * dimmly.
 */
void	CalibrationDisplayWidget::drawDisabled(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw disabled state");
	drawCommon(painter, false, true);
}

/**
 * \brief Draw enabled state of the widget
 *
 * In the enabled state, vectors are drawn if present, and everything is bright
 */
void	CalibrationDisplayWidget::drawEnabled(QPainter& painter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw enabled state");
	drawCommon(painter, _calibration.complete, false);
}

/**
 * \brief Compute the effect of the calibration on an offset/time
 */
snowstar::Point	operator*(const snowstar::Calibration& calibration,
			const snowstar::CalibrationPoint& calibrationpoint) {
	snowstar::Point	result;
	if (!calibration.complete) {
		result.x = 0;
		result.y = 0;
		return result;
	}
	result.x = calibration.coefficients[0] * calibrationpoint.offset.x +
		calibration.coefficients[1] * calibrationpoint.offset.y +
		calibration.coefficients[2] * calibrationpoint.t;
	result.y = calibration.coefficients[3] * calibrationpoint.offset.x +
		calibration.coefficients[4] * calibrationpoint.offset.y +
		calibration.coefficients[5] * calibrationpoint.t;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "(%f,%f) -> (%f,%f)",
	//	calibrationpoint.offset.x, calibrationpoint.offset.y,
	//	result.x, result.y);
	return result;
}

/**
 * \brief Draw calibration points and vectors
 */
void	CalibrationDisplayWidget::drawCommon(QPainter& painter,
		bool drawvectors, bool dim) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing calibration %d, %d points",
		_calibration.id, _calibration.points.size());
	double	gray = (dim) ? 204. : 255.;
	painter.fillRect(0, 0, width(), height(), QColor(gray, gray, gray));

	// draw the coorrdinate system
	gray = (dim) ? 128. : 102.;
	painter.fillRect(width() / 2, 0, 1, height(), QColor(gray, gray, gray));
	painter.fillRect(0, height() / 2, width(), 1, QColor(gray, gray, gray));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "coordinate system drawn");
	if (_calibration.id < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop drawing, no cal");
		return;
	}

	// get the first point as a reference
	snowstar::Point	ref;
	ref.x = 0;
	ref.y = 0;
	switch (_calibration.type) {
	case snowstar::ControlGuidePort:
		if (_calibration.points.size() > 0) {
			snowstar::CalibrationPoint p = _calibration.points[0];
			ref.x = p.star.x;
			ref.y = p.star.y;
		}
		break;
	case snowstar::ControlAdaptiveOptics:
		{
		for (unsigned int i = 0; i < _calibration.points.size(); i++) {
			ref.x += _calibration.points[i].star.x;
			ref.y += _calibration.points[i].star.y;
		}
		ref.x /= _calibration.points.size();
		ref.y /= _calibration.points.size();
		}
		break;
	}

	// determine the coordinate system scale, for this we first need
	// some parameters
	double	timeinterval = 0;
	int	counter = 0;
	double	maxx = 10;	// 10 pixels
	double	maxy = 10;	// 10 pixels
	for (unsigned long i = 0; i < _calibration.points.size(); i++) {
		snowstar::CalibrationPoint	p = _calibration.points[i];

		// compute the time interval used
		if (p.offset.x != 0) {
			timeinterval += fabs(p.offset.x); counter++;
		}
		if (p.offset.y != 0) {
			timeinterval += fabs(p.offset.y); counter++;
		}

		// compute the maximum coordinates
		double	x = fabs(p.star.x - ref.x);
		double	y = fabs(p.star.y - ref.y);
		if (x > maxx) { maxx = x; }
		if (y > maxy) { maxy = y; }
	}
	if (counter > 0) {
		timeinterval /= counter;
	} else {
		timeinterval = 1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d intervals seen, interval = %f",
		counter, timeinterval);

	double	rax = 0, ray = 0, decx = 0, decy = 0, driftx = 0, drifty = 0;
	if (_calibration.complete) {
		if (_calibration.coefficients.size() != 6) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "6 != %d coefficients",
				_calibration.coefficients.size());
			return;
		}
		rax = _calibration.coefficients[0] * timeinterval;
		ray = _calibration.coefficients[3] * timeinterval;
		if (fabs(rax) > maxx) { maxx = fabs(rax); }
		if (fabs(ray) > maxy) { maxy = fabs(ray); }

		decx = _calibration.coefficients[1] * timeinterval;
		decy = _calibration.coefficients[4] * timeinterval;
		if (fabs(decx) > maxx) { maxx = fabs(decx); }
		if (fabs(decy) > maxy) { maxy = fabs(decy); }

		driftx = _calibration.coefficients[2] * timeinterval;
		drifty = _calibration.coefficients[5] * timeinterval;
		if (fabs(driftx) > maxx) { maxx = fabs(driftx); }
		if (fabs(drifty) > maxy) { maxy = fabs(drifty); }
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA = %f/%f, DEC = %f/%f, t = %f/%f",
		rax, ray, decx, decy, driftx, drifty);

	// add 20% additional space
	maxx *= 1.2;
	maxy *= 1.2;

	double	scalex = (width() / 2.) / maxx;	// px / calpixel
	double	scaley = (height() / 2.) / maxy;
	double	scale = (scalex < scaley) ? scalex : scaley;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale = %f", scale);

	// center point
	double	cx = width() / 2;
	double	cy = height() / 2;
	double	h = height() - 1;
	h = height();
	QPointF	center(cx, h - cy);


	// we are going to draw a few things, we need a pen for that
	QPen	pen(Qt::SolidLine);

	// draw the coordinate grid (10 pixel lines)
	int	wm = floor(((width() / 2.) / scalex) / 10);
	int	hm = floor(((height() / 2.) / scaley) / 10);
	QColor	gridcolor(204, 204, 204);
	pen.setColor(gridcolor);
	pen.setWidth(1);
	painter.setPen(pen);
	for (int xi = -wm; xi <= wm; xi++) {
		double	x = cx + 10 * xi * scalex;
		painter.drawLine(x, 0, x, height());
	}
	for (int yi = -hm; yi <= hm; yi++) {
		double	y = cy + 10 * yi * scaley;
		painter.drawLine(0, y, width(), y);
	}

	// draw the points
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw points");
	pen.setWidth(3);
	QColor	pencolor = (dim) ? QColor(153., 102., 102.)
				 : QColor(255., 0., 0.);
	Color	red(pencolor);
	pen.setColor(pencolor);
	painter.setPen(pen);
	if (drawvectors) {
		for (unsigned long i = 0; i < _calibration.points.size(); i++) {
			// actual point
			snowstar::CalibrationPoint	p = _calibration.points[i];
			QPointF	pf((p.star.x - ref.x) * scale + cx,
					h - ((p.star.y - ref.y) * scale + cy));
			snowstar::Point	q = _calibration * p;
			QPointF	qf(q.x * scale + cx, h - (q.y * scale + cy));
			double	r = hypot(qf.x() - pf.x(), qf.y() - pf.y()) + 2;
			QPainterPath	path;
			path.addEllipse(qf, r, r);
			painter.fillPath(path, QBrush((red * 0.3).qcolor()));
		}
	}
	QFont	labelfont;
	labelfont.setPointSize(12);
	painter.setFont(labelfont);
	for (unsigned long i = 0; i < _calibration.points.size(); i++) {
		// actual point
		pen.setColor(pencolor);
		painter.setPen(pen);
		snowstar::CalibrationPoint	p = _calibration.points[i];
		QPointF	relpoint((p.star.x - ref.x) * scale,
				(p.star.y - ref.y) * scale);
		QPointF	pf(relpoint.x() + cx, h - (relpoint.y() + cy));
		painter.drawPoint(pf);

		// display the label if the point is far enough away
		double	d = hypot(relpoint.x(), relpoint.y());
		double	s = (d + 10) / d;
		if ((d > 15) && (_pointlabels)) {
			pen.setColor(QColor(0, 0, 0));
			painter.setPen(pen);
			QPointF	labelpoint(s * relpoint.x() + cx - 10,
					h - (s * relpoint.y() + cy) - 10);
			char	l[10];
			snprintf(l, sizeof(l), "%lu", i);
			painter.drawText(labelpoint.x(), labelpoint.y(),
				20, 20, Qt::AlignCenter, QString(l));
		}
	}
	
	if (!drawvectors) {
		return;
	}

	// draw the vectors
	pen.setWidth(2);
	QFont	font;
	painter.setFont(font);

	// draw R vector
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw R vector");
	pen.setColor((dim) ? QColor(51., 51., 102.) : QColor(0., 0., 204.));
	painter.setPen(pen);
	QPointF	ra(rax * scale + cx, h - (ray * scale + cy));
	painter.drawLine(center, ra);
	double	r = hypot(rax, ray) * scale;
	r = scale * (r + 10) / r;
	painter.drawText(rax * r + cx - 10, h - (ray * r + cy) - 10, 20, 20,
		Qt::AlignCenter, QString("R"));

	// draw D vector
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw D vector");
	pen.setColor((dim) ? QColor(102., 204., 153.) : QColor(0., 102., 51.));
	painter.setPen(pen);
	QPointF	dec(decx * scale + cx, h - (decy * scale + cy));
	painter.drawLine(center, dec);
	r = hypot(decx, decy) * scale;
	r = scale * (r + 10) / r;
	painter.drawText(decx * r + cx - 10, h - (decy * r + cy) - 10, 20, 20,
		Qt::AlignCenter, QString("D"));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw t vector");
	pen.setColor((dim) ? QColor(204., 153., 102.) : QColor(255., 153., 51.));
	painter.setPen(pen);
	QPointF	drift(driftx * scale + cx, h - (drifty * scale + cy));
	painter.drawLine(center, drift);
	r = hypot(driftx, drifty) * scale;
	r = scale * (r + 10) / r;
	painter.drawText(driftx * r + cx - 10, h - (drifty * r + cy) - 10,
		20, 20, Qt::AlignCenter, QString("t"));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing complete");
}

/**
 * \brief Slot to handle changes in hiddenness
 */
void	CalibrationDisplayWidget::changeEvent(QEvent *event) {
	// check for events that 
	if (event->type() == QEvent::EnabledChange) {
		repaint();
	}
	QWidget::changeEvent(event);
}

} // namespace snowgui
