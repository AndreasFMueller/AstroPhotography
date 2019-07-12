/*
 * DitherWork.cpp
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
 * \brief Construct a DitherWork object
 */
DitherWork::DitherWork(TaskQueueEntry& task) : TaskWork(task) {
	if (task.taskType() != tasktype(tasktype::DITHER)) {
		std::string	msg = stringprintf("%d is not a dither task",
			task.id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::logic_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct dither task work object %s",
		task.toString().c_str());
}

/**
 * \brief Destroy the DitherWork object
 */
DitherWork::~DitherWork() {
}

/**
 * \brief Do the sleep work, i.e. do sleep
 */
void	DitherWork::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start to dither task");
	try {
		// XXX do the work a dither task has to do
	} catch (CancelException& cancelexception) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dither %d task cancelled",
			task().id());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end dither task");
}

} // namespace task
} // namespace astro
