/*
 * TaskQueue.cpp -- queue of tasks
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>

namespace astro {
namespace task {

static void	*taskmain(void *p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskmain started: %p", p);
	TaskExecutor	*te = (TaskExecutor *)p;
	te->main(p);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskmain complete: %p", p);
}

void	TaskExecutor::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main started");

	// get camera and ccd

	// turn on the cooler

	// get the filterwheel

	// start exposure
	_task.state(pending);
	_queue.update(_task);

	// wait for completion of exposure
	usleep(10000000);

	// read image, add to the ImageDirectory
	

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main terminated");
}

TaskExecutor::TaskExecutor(const TaskQueueEntry& task) : _task(task) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a new executor");
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_create(&_thread, &attr, taskmain, this);
	pthread_attr_create(&attr);
}

TaskExecutor::~TaskExecutor() {
}

TaskQueue::TaskQueue() {
}

} // namespace task
} // namespace astro
