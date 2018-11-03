/*
 * LocalTimeWidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <LocalTimeWidget.h>
#include <time.h>

namespace snowgui {

LocalTimeWidget::LocalTimeWidget(QWidget *parent) : QLabel(parent) {
	connect(&_statusTimer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));
	_statusTimer.setInterval(1000);
	_statusTimer.start();
}

LocalTimeWidget::~LocalTimeWidget() {
	_statusTimer.stop();
}

void	LocalTimeWidget::statusUpdate() {
	time_t	now;
	time(&now);
	struct tm	*tmp = localtime(&now);
	char	buffer[32];
	strftime(buffer, sizeof(buffer), "%H:%M:%S", tmp);
	setText(QString(buffer));
}

} // namespace snowgui
