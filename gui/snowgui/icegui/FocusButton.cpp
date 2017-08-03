/*
 * FocusButton.cpp -- Implementation of the focus button
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusButton.h>
#include <AstroDebug.h>
#include <QPainter>
#include <cmath>

namespace snowgui {

FocusButton::FocusButton(QWidget *parent) : QPushButton(parent) {
	_f = 1.5;
}

FocusButton::~FocusButton() {
}

void	FocusButton::paintEvent(QPaintEvent * /* event */) {
	draw();
}

void	FocusButton::draw() {
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// dark gray background
	QColor	gray(127, 127, 127);
	painter.fillRect(0, 0, width(), height(), gray);

	// parameters
	double	lensx = width() * 0.2;
	double	focusx = width() * (0.8 + 0.1 * sin(_f));
	double	f = focusx - lensx;
	double	h = 0.9 * height() / 2.;
	double	F = hypot(f, h);
	double	angle = 180 * atan(h / f) / M_PI;

	// draw the lens
	QPainterPath	lens;
	lens.moveTo(QPointF(lensx, height() / 2. - h));
	lens.arcTo(lensx + f - F - 1, height() / 2. - F, 2 * F, 2 * F,
		180 - angle, 2 * angle);
	lens.arcTo(lensx - f - F + 1, height() / 2. - F, 2 * F, 2 * F,
		-angle, 2 * angle);
	lens.closeSubpath();
	QColor	lenscolor(204., 204., (isEnabled()) ? 255. : 204.);
	painter.fillPath(lens, lenscolor);

	// draw the light rays
	QColor	raycolor(255., 255., (isEnabled()) ? 0. : 255.);
	double	dy = h / 6;
	QPen	pen(Qt::SolidLine);
	pen.setWidth(2);
	pen.setColor(raycolor);
	painter.setPen(pen);
	for (int i = -5; i <= 5; i += 2) {
		double	y = i * dy + height() / 2.;
		painter.drawLine(QPointF(0, y), QPointF(lensx, y));
		double	Y = height() / 2. - ((width() - focusx) / f) * i * dy;
		painter.drawLine(QPointF(lensx, y), QPointF(width(), Y));
	}
}

void	FocusButton::update(double f) {
	_f = f;
	repaint();
}

} // namespace snowgui
