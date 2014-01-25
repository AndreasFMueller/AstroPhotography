/*
 * calibrationwidget.cpp -- Widget to display the calibration as a pair of
 *                          vectors
 *
 * (c) 2014 Prof Dr Andreas Mueller; Hochschule Rapperswil
 */
#include <calibrationwidget.h>
#include <QPainter>
#include <cmath>
#include <AstroDebug.h>

CalibrationWidget::CalibrationWidget(QWidget *parent) : QWidget(parent) {
	setToolTip("Calibration vector representation:\n"
		"translations caused on the CCD by\n"
		"1 second RA and DEC activations");
	setToolTipDuration(10000);
}

CalibrationWidget::~CalibrationWidget() {
}

void	CalibrationWidget::addCalibration(const Astro::Calibration& calibration) {
	ravector.first = calibration.coefficients[0];
	ravector.second = calibration.coefficients[3];
	decvector.first = calibration.coefficients[1];
	decvector.second = calibration.coefficients[4];
	driftvector.first = calibration.coefficients[2];
	driftvector.second = calibration.coefficients[5];
}

void	CalibrationWidget::draw() {
	// get a Painter
	QPainter	painter(this);
	painter.fillRect(0, 0, width(), height(), QColor(255., 255., 255.));
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	pen.setColor(QColor(0., 0., 255.));
	painter.setPen(pen);

	// find the optimal scale
	double	xscale = 1;
	double	yscale = 1;
	double	x = fabs(ravector.first);
	if (x > xscale) { xscale = x; }
	double	y = fabs(ravector.second);
	if (y > yscale) { yscale = y; }

	x = fabs(decvector.first);
	if (x > xscale) { xscale = x; }
	y = fabs(decvector.second);
	if (y > yscale) { yscale = y; }

	x = fabs(driftvector.first);
	if (x > xscale) { xscale = x; }
	y = fabs(driftvector.second);
	if (y > yscale) { yscale = y; }

	xscale = (width() - 4) / (2 * xscale);
	yscale = (height() - 4) / (2 * yscale);
	double	scale = (xscale > yscale) ? yscale : xscale;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scale = %f", scale);

	// draw the coordinate system
	painter.fillRect(width() / 2, 0, 1, height(), QColor(128., 128., 128.));
	painter.fillRect(0, height() / 2, width(), 1, QColor(128., 128., 128.));

	// draw the vectors
	pen.setColor(QColor(0., 0., 255.));
	painter.setPen(pen);
	QPointF	center(width() / 2, height() / 2);
	QPointF	rapoint(center.x() + scale * ravector.first,
			center.y() - scale * ravector.second);
	painter.drawLine(center, rapoint);
	painter.drawText(rapoint.x() - 10, rapoint.y() - 10, 20, 20,
		Qt::AlignCenter, QString("R"));

	pen.setColor(QColor(0., 128., 0.));
	painter.setPen(pen);
	QPointF	decpoint(center.x() + scale * decvector.first,
			center.y() - scale * decvector.second);
	painter.drawLine(center, decpoint);
	painter.drawText(decpoint.x() - 10, decpoint.y() - 10, 20, 20,
		Qt::AlignCenter, QString("D"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rapoint = %f,%f",
		rapoint.x(), rapoint.y());

	// draw the vectors
}

void	CalibrationWidget::paintEvent(QPaintEvent *event) {
	draw();
}

