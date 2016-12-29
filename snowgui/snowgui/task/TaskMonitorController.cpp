/*
 * TaskMonitorController.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "TaskMonitorController.h"
#include <CommunicatorSingleton.h>
#include <AstroDebug.h>

namespace snowgui {

TaskMonitorController::TaskMonitorController(QObject *parent)
	: QObject(parent) {
	qRegisterMetaType<snowstar::TaskMonitorInfo>("snowstar::TaskMonitorInfo");
}

TaskMonitorController::~TaskMonitorController() {
	if (_tasks) {
		_tasks->unregisterMonitor(_myidentity);
	}
}

void	TaskMonitorController::setTasks(snowstar::TaskQueuePrx tasks,
		Ice::ObjectPtr myself) {
	_tasks = tasks;
	if (!_tasks) {
		return;
	}
	snowstar::CommunicatorSingleton::connect(_tasks);
	_myidentity = snowstar::CommunicatorSingleton::add(myself);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "identity: %s",
		_myidentity.name.c_str());
	_tasks->registerMonitor(_myidentity);
}

void	TaskMonitorController::stop(const Ice::Current&) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
}

void	TaskMonitorController::update(const snowstar::TaskMonitorInfo& info,
	const Ice::Current&) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task %d new state %d %f ago",
		info.taskid, info.newstate, info.timeago);
	emit taskUpdate(info);
}

} // namespace snowgui
