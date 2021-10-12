/*
 * EastWestIndicator.cpp -- indicator to show the current task queue status
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "EastWestIndicator.h"
#include <QPainter>
#include <QPainterPath>
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Create a TaskIndicator
 */
EastWestIndicator::EastWestIndicator(QWidget *parent) : QWidget(parent) {
	_north = true;
	_east = false;
}

/**
 * \brief Destroy the task indicator
 */
EastWestIndicator::~EastWestIndicator() {
}

/**
 * \brief Slot to update the current state
 */
void	EastWestIndicator::update(bool east) {
	_east = east;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "position update: new position %s",
		(_east) ? "east" : "west");
	repaint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repaint() complete");
}

/**
 * \brief Event handler to redraw the task indicator
 */
void	EastWestIndicator::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Draw the task indicator
 */
void	EastWestIndicator::draw() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the current position");
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	float	r = height()/2 - 1;
	if ((6 * r) > width()) {
		r = width() / 6;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using r = %.1f", r);

	// draw the DEC axis
	{
		QColor	axiscolor(204., 204., 204.);
		QPen	pen(Qt::SolidLine);
		pen.setWidth(3);
		pen.setColor(axiscolor);
		painter.setPen(pen);
		QLineF	dec_axis(1., height()/2., width()-1., height()/2.);
		painter.drawLine(dec_axis);
	}

	// draw the RA axis
	{
		float	R = 3;
		QPainterPath	path;
		path.moveTo(width()/2-R,height()/2);
		path.lineTo(width()/2-R-1,height());
		path.lineTo(width()/2+R+1,height());
		path.lineTo(width()/2+R,height()/2);
		painter.fillPath(path, Qt::black);

		R = 6;
		double	r = R / sqrt(2);
		QRectF	rect(width()/2-R,height()/2-r,2*R,2*r);
		QPen	pen(Qt::SolidLine);
		pen.setWidth(0);
		pen.setColor(Qt::black);
		painter.setPen(pen);
		painter.setBrush(Qt::black);
		painter.drawEllipse(rect);

		R = 0.7 * R; r = R / sqrt(2);
		rect = QRectF(width()/2-R,height()/2-r,2*R,2*r);
		pen.setWidth(0);
		pen.setColor(Qt::white);
		painter.setPen(pen);
		painter.setBrush(Qt::white);
		painter.drawEllipse(rect);

		R = 2; r = R/sqrt(2);
		rect = QRectF(width()/2-R,height()/2-r,2*R,2*r);
		pen.setWidth(0);
		pen.setColor(Qt::black);
		painter.setPen(pen);
		painter.setBrush(Qt::black);
		painter.drawEllipse(rect);
	}

	bool	telescope_right = _east ^ _north;

	// draw a circle representing the telescope
	{
		QPointF	center((telescope_right) ? (width()-r-1) : r+1,
				height() / 2);
		QRectF	rect(center.x() - r, center.y() - r, 2 * r, 2 * r);
		QPen	pen(Qt::SolidLine);
		pen.setWidth(1);
		pen.setColor(Qt::black);
		painter.setPen(pen);
		painter.setBrush(Qt::white);
		painter.drawEllipse(rect);
		float	s = sqrt(2);
		QRectF	textrect(center.x() - 0.8*r/s, center.y() - 0.8*r/s,
				0.8*s * r, 0.8*s * r);
		QFont	font = painter.font();
		font.setPixelSize(s*r);
		painter.setFont(font);
		painter.drawText(textrect, Qt::AlignCenter,
			(_east) ? QString("E") : QString("W"));
	}

	// draw the counterweights
	{
		QRectF	rect1((telescope_right) ? 2 : (width() - 4 - 2),
				height()/2 - 5,
				3, 10);
		painter.fillRect(rect1, Qt::black);
		QRectF	rect2((telescope_right) ? 7 : (width() - 8 - 2),
				height()/2 - 5,
				3, 10);
		painter.fillRect(rect2, Qt::black);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw() complete");
}

} // namespace snowgui
