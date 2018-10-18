/*
 * StreamWork.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"
#include <QTimer>

namespace snowgui {

StreamWork::StreamWork(LiveView *liveview)
	: QObject(NULL), _liveview(liveview) {
}

StreamWork::~StreamWork() {
}

void	StreamWork::stop() {
	_running = false;
}

void	StreamWork::start() {
	_running = true;
	nextExposure();
}

void	StreamWork::nextExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nextExposure()");
	_liveview->doExposure();
	// rearm the timer
	if (_running) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "rearming the timer");
		QTimer::singleShot(_interval, this, SLOT(nextExposure()));
	}
}

void	StreamWork::interval(double t) {
	_interval = 1000 * t;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new interval value: %d", _interval);
}


} // namespace snowgui
