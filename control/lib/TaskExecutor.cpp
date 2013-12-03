/*
 * TaskExecutor.cpp -- queue of tasks
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>

using namespace astro::persistence;

namespace astro {
namespace task {

/**
 * \brief Springboard function to start the main method of the Executor class
 */
static void	*taskmain(void *p) {
	TaskExecutor	*te = (TaskExecutor *)p;
	te->main();
	return te;
}

/**
 * \brief Timed wait function
 *
 * The result indicates whether the wait was completed or the wait was
 * interrupted. If true, the wait completed.
 */
bool	TaskExecutor::wait(float t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for %.3f seconds", t);
	int	seconds = floor(t);
	int	nanoseconds = 1000000000 * (t - seconds);
	struct timeval	tv;
	struct timespec	ts;
	gettimeofday(&tv, NULL);
	ts.tv_sec = tv.tv_sec + floor(t);
	ts.tv_nsec = tv.tv_usec + nanoseconds;
	if (ts.tv_nsec > 999999999) {
		ts.tv_sec += ts.tv_nsec / 1000000000;
		ts.tv_nsec = ts.tv_nsec % 1000000000;
	}
	
	// wait for completion of exposure
	int	rc = pthread_cond_timedwait(&_cond, &_lock, &ts);
	if (rc == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait cancelled");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait complete: %s (%d)",
			strerror(rc), rc);
	}
	return (rc == ETIMEDOUT) ? true : false;
}

/**
 * \brief Task Executor main function
 *
 * Special consideration is needed for cancelling such a thread. If the
 * exposure has already begun, cancelling must extend to the exposure
 * that must be cancelled.
 */
void	TaskExecutor::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main started");
	pthread_mutex_lock(&_lock);
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "entering main task region");
		// inform queue of state change
		_task.state(TaskQueueEntry::executing);
		_queue.post(_task.id()); // notify queue of state change
		
		// XXX get camera and ccd
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera '%s' and ccd %d",
			_task.camera().c_str(), _task.ccdid());

		// XXX turn on the cooler
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set cooler to %f",
			_task.ccdtemperature());

		// XXX get the filterwheel
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get filter %d of wheel '%s'",
			_task.filterposition(), _task.filterwheel().c_str());

		// XXX start exposure
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure");

		// wait for completion of exposure
		if (wait(_task.exposure().exposuretime)) {
			// read image, add to the ImageDirectory
			_task.filename(stringprintf("file%d.fits", _task.id()));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s written",
				_task.filename().c_str());
			_task.state(TaskQueueEntry::complete);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "thread cancelled");
			_task.state(TaskQueueEntry::cancelled);
		}
	} catch (...) {
		_task.state(TaskQueueEntry::failed);
	}
	_queue.post(_task.id());
	pthread_mutex_unlock(&_lock);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main terminated");
}

/**
 * \brief create a task executor
 *
 * launch a new thread to work an a given task
 * \param queue		the queue that ons this executor
 * \param task		the task queue entry describing the task
 */
TaskExecutor::TaskExecutor(TaskQueue& queue, const TaskQueueEntry& task)
	: _queue(queue), _task(task) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a new executor");

	// initialize pthead resources
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutex_init(&_lock, &mattr);
	pthread_mutex_lock(&_lock);

	pthread_condattr_t	cattr;
	pthread_condattr_init(&cattr);
	pthread_cond_init(&_cond, &cattr);

	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_create(&_thread, &attr, taskmain, this);
	pthread_attr_destroy(&attr);

	// release the lock
	pthread_mutex_unlock(&_lock);
}

/**
 * \brief destroy a task exectuor
 */
TaskExecutor::~TaskExecutor() {
	cancel();
	wait();
}

/**
 * \brief cancel execution 
 */
void	TaskExecutor::cancel() {
	pthread_cond_signal(&_cond);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread cancel signal sent");
}

/**
 * \brief wait for the thread to terminate
 */
void	TaskExecutor::wait() {
	void	*result;
	int	rc = pthread_join(_thread, &result);
	if (rc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error during thread join: %s",
			strerror(rc));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread terminated");
}

/**
 * \brief check whether this executor blocks a given task queue entry
 */
bool	TaskExecutor::blocks(const TaskQueueEntry& other) {
	return _task.blocks(other);
}

} // namespace task
} // namespace astro
