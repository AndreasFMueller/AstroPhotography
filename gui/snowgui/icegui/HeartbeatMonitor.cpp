/*
 * HeartbeatMonitor.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswi
 */
#include "systeminfowidget.h"
#include <types.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroTypes.h>

namespace snowgui {

/**
 * \brief Create a new heartbeat monitor
 */
HeartbeatMonitor::HeartbeatMonitor() : QObject(NULL) {
	_multiplier = 5;
	_timer.setInterval(0);
	_timer.setSingleShot(true);
	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(timeout()), Qt::QueuedConnection);
	connect(this, SIGNAL(start_timer_signal(int)),
		this, SLOT(start_timer(int)), Qt::QueuedConnection);
	connect(this, SIGNAL(stop_timer_signal()),
		this, SLOT(stop_timer()), Qt::QueuedConnection);
	_lost = false;
}

/**
 * \brief Destroy the monitor
 */
HeartbeatMonitor::~HeartbeatMonitor() {
	emit stop_timer_signal();
}

/**
 * \brief Auxiliary function to compute the wait time
 */
int	HeartbeatMonitor::milliseconds() const {
	int	milliseconds = 1000 * _interval * _multiplier;
	if (_lost) {
		milliseconds *= 2;
	}
	return milliseconds;
}

/**
 * \brief Handle the beat callback interface
 *
 * \param sequence_number	the sequence number
 * \param current		the current call context
 */
void	HeartbeatMonitor::beat(int sequence_number,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sequence_number = %d", sequence_number);

	// construct a timestamp
	time_t	now;
	time(&now);
	char	buffer[128];
	struct tm	*timep = localtime(&now);
	strftime(buffer, sizeof(buffer), "%T %F", timep);
	std::string	l = astro::stringprintf("%s, seqno = %d", buffer,
		sequence_number);

	// if the connection was lost, send the reconnect signal
	if (_lost) {
		_lost = false;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "emit reconnected signal");
		emit reconnected();
	}

	// restart the timer
	if (_interval > 0) {
		emit start_timer_signal(milliseconds());
	}

	// emit the timestamp
	emit update(QString(l.c_str()));
}

/**
 * \brief Handle the interval callback interface
 *
 * \param intvl		the new interval
 * \param current	the current call context
 */
void	HeartbeatMonitor::interval(float intvl,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new interval received: %f", intvl);
	_interval = intvl;
	if (milliseconds() > 0) {
		emit start_timer_signal(milliseconds());
	}
}

/**
 * \brief handle the stop method
 *
 * \param current	the current call context
 */
void	HeartbeatMonitor::stop(const Ice::Current& /* current */) {
	emit stop_timer_signal();
	_lost = true;
	emit lost();
}

/**
 * \brief Slot called when the timer times out
 */
void	HeartbeatMonitor::timeout() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server lost");
	_lost = true;
	emit lost();
}

/**
 * \brief Try to set the multiplier
 *
 * This method throws an exception if the multiplier ist not at least 1
 */
void	HeartbeatMonitor::multiplier(int m) {
	if (m < 1) {
		std::string	msg = astro::stringprintf("multiplier must be "
			">= 1, %d specified", m);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_multiplier = m;
}

/**
 * \brief Slot to actually stop the timer
 */
void	HeartbeatMonitor::stop_timer() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop timer");
	_timer.stop();
}

/**
 * \brief Slot to actually start the timer
 */
void	HeartbeatMonitor::start_timer(int milliseconds) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"starting timer with %d milliseconds", milliseconds);
	_timer.start(std::chrono::milliseconds(milliseconds));
}

} // namespace snowgui
