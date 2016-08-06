/*
 * CalibrationDisplayWidget.cpp -- implementation of the calibration display
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CalibrationDisplayWidget.h>
#include <QPainter>
#include <AstroDebug.h>
#include <cmath>

namespace snowgui {

CalibrationDisplayWidget::CalibrationDisplayWidget(QWidget *parent)
	: QWidget(parent) {
	_calibration.id = -1;
	_calibration.complete = false;
}

CalibrationDisplayWidget::~CalibrationDisplayWidget() {
}

void	CalibrationDisplayWidget::setCalibration(snowstar::Calibration calibration) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new calibration: %d", calibration.id);
	_calibration = calibration;
	repaint();
}

void	CalibrationDisplayWidget::paintEvent(QPaintEvent * /* event */) {
	draw();
}

void	CalibrationDisplayWidget::draw() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "drawing calibration %d",
		_calibration.id);
	QPainter	painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));
	QPen	pen(Qt::SolidLine);
	pen.setWidth(3);
	pen.setColor(QColor(255., 0., 0.));
	painter.setPen(pen);

	// draw the coorrdinate system
	painter.fillRect(width() / 2, 0, 1, height(), QColor(128., 128., 128.));
	painter.fillRect(0, height() / 2, width(), 1, QColor(128., 128., 128.));

	// get the first point as a reference
	snowstar::Point	ref;

	// determine the coordinate system scale, for this we first need
	// some parameters
	double	timeinterval = 0;
	double	counter = 0;
	double	maxx = 1;
	double	maxy = 1;
	for (unsigned long i = 0; i < _calibration.points.size(); i++) {
		snowstar::CalibrationPoint	p = _calibration.points[i];
		if (i == 0) { ref.x = p.star.x; ref.y = p.star.y; }

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

	double	rax, ray, decx, decy, driftx, drifty;
	if (_calibration.complete) {
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

	// add 20% additional space
	maxx *= 1.2;
	maxy *= 1.2;

	double	scalex = (width() / 2.) / maxx;
	double	scaley = (height() / 2.) / maxy;
	double	scale = (scalex < scaley) ? scalex : scaley;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale = %f", scale);

	// center point
	double	cx = width() / 2;
	double	cy = height() / 2;
	double	h = height() - 1;

	// draw the points
	pen.setColor(QColor(255., 0., 0.));
	painter.setPen(pen);
	for (unsigned long i = 0; i < _calibration.points.size(); i++) {
		snowstar::CalibrationPoint	p = _calibration.points[i];
		QPointF	pf((p.star.x - ref.x) * scale + cx,
			h - ((p.star.y - ref.y) * scale + cy));
		painter.drawPoint(pf);
	}
	
	if (!_calibration.complete) {
		return;
	}

	// draw the vectors
	QPointF	center(cx, cy);
	pen.setWidth(2);

	pen.setColor(QColor(0., 0., 204.));
	painter.setPen(pen);
	QPointF	ra(rax * scale + cx, h - (ray * scale + cy));
	painter.drawLine(center, ra);
	double	r = hypot(rax, ray) * scale;
	r = scale * (r + 10) / r;
	painter.drawText(rax * r + cx - 10, h - (ray * r + cy) - 10, 20, 20,
		Qt::AlignCenter, QString("R"));

	pen.setColor(QColor(0., 102., 51.));
	painter.setPen(pen);
	QPointF	dec(decx * scale + cx, h - (decy * scale + cy));
	painter.drawLine(center, dec);
	r = hypot(decx, decy) * scale;
	r = scale * (r + 10) / r;
	painter.drawText(decx * r + cx - 10, h - (decy * r + cy) - 10, 20, 20,
		Qt::AlignCenter, QString("D"));

	pen.setColor(QColor(255., 153., 51.));
	painter.setPen(pen);
	QPointF	drift(driftx * scale + cx, h - (drifty * scale + cy));
	painter.drawLine(center, drift);
	r = hypot(driftx, drifty) * scale;
	r = scale * (r + 10) / r;
	painter.drawText(driftx * r + cx - 10, h - (drifty * r + cy) - 10,
		20, 20, Qt::AlignCenter, QString("t"));
}

} // namespace snowgui
