/*
 * StreamWork.cpp -- implementation for StreamWork class
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "liveview.h"
#include <QTimer>

namespace snowgui {

/**
 * \brief Create the StreamWork class
 *
 * \param liveview	LiveView instance that does the actual work
 */
StreamWork::StreamWork(LiveView *liveview)
	: QObject(NULL), _liveview(liveview) {
}

/**
 * \brief Destroy the StreamWork object
 */
StreamWork::~StreamWork() {
}

/**
 * \brief Stop the stream
 */
void	StreamWork::stop() {
	_running = false;
}

/**
 * \brief Start the stream
 *
 * Starting the stream essentially means calling the nextExposure method,
 * in turn calls the doExposure() method of the remembered LiveView instance.
 * After the exposure is complete, the timer is armed to trigger a new
 * exposure after the _interval timeout.
 */
void	StreamWork::start() {
	_running = true;
	// invoke the nextExposure method in the thread of the StreamWork
	// instance
	QMetaObject::invokeMethod(this, "nextExposure", Qt::QueuedConnection);
}

/**
 * \brief NextExposure slot
 */
void	StreamWork::nextExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nextExposure()");
	_liveview->doExposure();
	// rearm the timer
	if (_running) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "rearming the timer");
		QTimer::singleShot(_interval, this, SLOT(nextExposure()));
	}
}

/**
 * \brief Slot used to change the interval
 *
 * \param t	new interval length
 */
void	StreamWork::interval(double t) {
	_interval = 1000 * t;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new interval value: %d", _interval);
}


} // namespace snowgui
