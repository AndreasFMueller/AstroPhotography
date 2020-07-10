/*
 * TasksIndicator.cpp -- indicator to show the current task queue status
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "TasksIndicator.h"
#include <QPainter>
#include <QPainterPath>
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Create a TaskIndicator
 */
TasksIndicator::TasksIndicator(QWidget *parent) : QWidget(parent) {
	_state = snowstar::QueueIDLE;
}

/**
 * \brief Destroy the task indicator
 */
TasksIndicator::~TasksIndicator() {
}

/**
 * \brief Slot to update the current state
 */
void	TasksIndicator::update(snowstar::QueueState state) {
	_state = state;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state update: new state=%d", state);
	repaint();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repaint() complete");
}

/**
 * \brief Event handler to redraw the task indicator
 */
void	TasksIndicator::paintEvent(QPaintEvent * /* event */) {
	draw();
}

/**
 * \brief Draw the task indicator
 */
void	TasksIndicator::draw() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw the current state");
	QPainter	painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	// geometric stuff
	QPoint	center(width() / 2., height() / 2.);
	double	radius = width() / 2.;
	if (height() < width()) {
		radius = height() / 2.;
	}

	// fill background black
	QColor	background(0, 0, 0, 255);
	painter.fillRect(0, 0, width(), height(), background);

	// circle around indicator
	QColor	white(255, 255, 255);
	QPainterPath	border;
	border.addEllipse(center, radius - 2, radius - 2);
	painter.fillPath(border, white);

	// set up the pen
	QPen	pen(Qt::SolidLine);
	pen.setColor(white);
	painter.setPen(pen);

	// color indicator
	QPainterPath	indicator;
	indicator.addEllipse(center, radius - 4, radius - 4);
	switch (_state) {
	case snowstar::QueueIDLE:
		painter.fillPath(indicator, QColor(204, 204, 204));
		painter.drawText(center.x() - 30, center.y() - 20, 60, 40,
			Qt::AlignCenter, QString("IDLE"));
		break;
	case snowstar::QueueLAUNCHING:
		painter.fillPath(indicator, QColor(0, 153, 0));
		painter.drawText(center.x() - 30, center.y() - 20, 60, 40,
			Qt::AlignCenter, QString("LAUNCH"));
		break;
	case snowstar::QueueSTOPPING:
		painter.fillPath(indicator, QColor(255, 153, 0));
		painter.drawText(center.x() - 30, center.y() - 20, 60, 40,
			Qt::AlignCenter, QString("STOPPING"));
		break;
	case snowstar::QueueSTOPPED:
		painter.fillPath(indicator, QColor(204, 0, 0));
		painter.drawText(center.x() - 30, center.y() - 20, 60, 40,
			Qt::AlignCenter, QString("STOPPED"));
		break;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "draw() complete");
}

} // namespace snowgui
