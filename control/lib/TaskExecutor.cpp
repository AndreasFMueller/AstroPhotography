/*
 * TaskExecutor.cpp -- queue of tasks
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <AstroLoader.h>
#include <AstroDevaccess.h>
#include <ImageDirectory.h>
#include <errno.h>
#include <string.h>
#include <AstroIO.h>
#include <ExposureWork.h>

using namespace astro::persistence;
using namespace astro::io;

namespace astro {
namespace task {

/**
 * \brief Springboard function to start the main method of the Executor class
 */
static void	taskmain(TaskExecutor *te) {
	te->main();
}

/**
 * \brief Task Executor main function
 *
 * Special consideration is needed for cancelling such a thread. If the
 * exposure has already begun, cancelling must extend to the exposure
 * that must be cancelled.
 */
void	TaskExecutor::main() {
	// acquiring the lock ensures that the new thread is blocked until
	// the starting thread is ready to release it.
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main started LOCK");
	std::unique_lock<std::mutex>	lock(_lock);
	// by sending the notification, we signal to the starting thread
	// that we are now up and running
	_cond.notify_all();
	lock.unlock();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "lock release UNLOCK");

	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "entering main task region");
		// inform queue of state change
		_task.state(TaskQueueEntry::executing);
		_queue.post(_task.id()); // notify queue of state change
		debug(LOG_DEBUG, DEBUG_LOG, 0, "update posted");

		// we signal to the constructor that the thread is now fully
		// running, with the correct state set in the task queue
		_cond.notify_one();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "constructor signaled");

		// the exposure task starts to run when we call the run method
		// this is also the moment when the 
		exposurework->run();
	} catch (CancelException& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "execution cancelled: %s",
			x.what());
		_task.state(TaskQueueEntry::cancelled);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "executor failure: %s", x.what());
		_task.state(TaskQueueEntry::failed);
	}

	// post the curretn state to the queue
	_queue.post(_task.id());

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a new executor LOCK");
	// create the lock
	std::unique_lock<std::mutex>	lock(_lock);

	// create a new ExposureTask object. The ExposureTask contains
	// the logic to actually execute the task
	exposurework = new ExposureWork(*this, _task);

	// initialize pthead resources
	_thread = std::thread(taskmain, this);
}

void	TaskExecutor::release() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "release executor LOCK");
	std::unique_lock<std::mutex>	lock(_lock);

	// now wait for the condition variable to be signaled. This will
	// indicate that the thread has indeed started running. Doing this
	// ensures that the TaskExecutor thread is running and everything in
	// the new state before the constructor returns. The wait call 
	// atomically unlocks the lock and waits for the condition variable
	// to be signaled.
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting releases lock: UNLOCK");
	_cond.wait(lock);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait completion locks: LOCK");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task executor now released UNLOCK");
}

/**
 * \brief destroy a task exectuor
 */
TaskExecutor::~TaskExecutor() {
	cancel();
	wait();
	// when we have waited for the thread, we are sure the exposurework
	// is no longer needed, and can be detroyed
	delete exposurework;
	exposurework = NULL;
}

/**
 * \brief cancel execution 
 */
void	TaskExecutor::cancel() {
	exposurework->cancel();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread cancel signal sent");
}

/**
 * \brief wait for the thread to terminate
 */
void	TaskExecutor::wait() {
	try {
		_thread.join();
	} catch (...) {
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
