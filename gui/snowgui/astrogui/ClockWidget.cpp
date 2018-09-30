/*
 * ClockWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ClockWidget.h"
#include <time.h>

namespace snowgui {

ClockWidget::ClockWidget(QWidget *parent) : QLabel(parent) {
	_offset = 0;
	_timer = new QTimer();
	_timer->setInterval(1000);
	connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
	_timer->start();
}

ClockWidget::~ClockWidget() {
	_timer->stop();
	delete _timer;
}

void	ClockWidget::update() {
	time_t	now;
	time(&now);
	struct tm	g;
	gmtime_r(&now, &g);
	char	buffer[1024];
	strftime(buffer, sizeof(buffer), "%T", &g);
	setText(QString(buffer));
}

} // namespace snowgui
