/*
 * SleepWork.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <ExposureWork.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <unistd.h>

namespace astro {
namespace task {

/**
 * \brief Construct a SleepWork object
 */
SleepWork::SleepWork(TaskQueueEntry& task) : TaskWork(task) {
	if (task.taskType() != tasktype(tasktype::SLEEP)) {
		std::string	msg = stringprintf("%d is not a sleep task",
			task.id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::logic_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct sleep task work object %s",
		task.toString().c_str());
	_sleeptime = task.exposure().exposuretime();
}

/**
 * \brief Destroy the SleepWork object
 */
SleepWork::~SleepWork() {
}

/**
 * \brief Do the sleep work, i.e. do sleep
 */
void	SleepWork::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start to sleep task, %.1f seconds",
		_sleeptime);
	try {
		wait(_sleeptime);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "task slept for %.1f seconds",
			_sleeptime);
	} catch (CancelException& cancelexception) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep %d task cancelled",
			task().id());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end sleep task");
}

} // namespace task
} // namespace astro
