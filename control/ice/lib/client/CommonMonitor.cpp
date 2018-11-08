/*
 * CommonMonitor.cpp -- base class for monitors
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommonClientTasks.h>

namespace snowstar {

void    CommonMonitor::complete(bool c) {
	_complete = c;
	if (_complete) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration complete");
		std::unique_lock<std::mutex>	lock(mtx);
		cond.notify_one();
	}
}

CommonMonitor::CommonMonitor() : _complete(false) {
}

void    CommonMonitor::wait() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for completion");
	std::unique_lock<std::mutex>	lock(mtx);
	cond.wait(lock);
}

} // namespace snowstar
