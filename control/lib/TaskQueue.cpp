/*
 * TaskQueue.cpp -- queue of tasks
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <unistd.h>
#include <pthread.h>

namespace astro {
namespace task {

//////////////////////////////////////////////////////////////////////
// TaskExecutor Implementation
//////////////////////////////////////////////////////////////////////
static void	*taskmain(void *p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskmain started: %p", p);
	TaskExecutor	*te = (TaskExecutor *)p;
	te->main();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskmain complete: %p", p);
	return te;
}

/**
 * \brief Task Executor main function
 */
void	TaskExecutor::main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main started");

	try {

		// get camera and ccd

		// turn on the cooler

		// get the filterwheel

		// start exposure
		_task.state(TaskQueueEntry::executing);
		_queue.update(_task);

		// XXX wait for completion of exposure
		usleep(10000000);

		// read image, add to the ImageDirectory
	

	} catch (...) {
		_task.state(TaskQueueEntry::failed);
		_queue.update(_task);
	}

	// tell the queue that this task has completed

	debug(LOG_DEBUG, DEBUG_LOG, 0, "main terminated");
}

/**
 * \brief create a task executor
 *
 * launch a new thread to work an a given task
 * \param queue		the queue that ons this executor
 * \param task		the task queue entry describing the task
 */
TaskExecutor::TaskExecutor(TaskQueue& queue, TaskQueueEntry& task)
	: _queue(queue), _task(task) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start a new executor");
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_create(&_thread, &attr, taskmain, this);
	pthread_attr_destroy(&attr);
}

/**
 * \brief destroy a task exectuor
 */
TaskExecutor::~TaskExecutor() {
	// don't continue until the thread has terminated
	pthread_join(_thread, NULL);
}

//////////////////////////////////////////////////////////////////////
// TaskQueue Implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief static main function for the task queue
 */
static void	*queuemain(void *p) {
	TaskQueue	*queue = (TaskQueue *)p;
	queue->main();
	return queue;
}

/**
 * \brief  main function for the task queue
 */
void	TaskQueue::main() {
	_running = true;
	pthread_mutex_lock(&_lock);
	while (_running) {
		// XXX process any tasks that have completed
		while (!_idqueue.empty()) {
			int	queueid = _idqueue.front();
			_idqueue.pop();
			cleanup(queueid);
		}

		// launch as many tasks as possible
		launch();

		if (pthread_cond_wait(&_cond, &_lock)) {
			throw std::runtime_error("pthread_cond_wait failed");
		}
	}
	_running = false;
}

/**
 * \brief start a task queue
 */
TaskQueue::TaskQueue() {
	pthread_mutexattr_t	mattr;
        pthread_mutexattr_init(&mattr);
	pthread_mutex_init(&_lock, &mattr);

	pthread_condattr_t	cattr;
	pthread_condattr_init(&cattr);
	pthread_cond_init(&_cond, &cattr);

	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_create(&_thread, &attr, queuemain, this);

	// launch new jobs
	launch();
}

/**
 * \brief Stop the task queue
 *
 * The destructor stops the thread, and destroys mutex lock and condition
 * variable
 */
TaskQueue::~TaskQueue() {
	if (_running) {
		throw std::runtime_error("has running executors");
	}
	pthread_cancel(_thread);
	pthread_cond_signal(&_cond);
	pthread_join(_thread, NULL);
	pthread_cond_destroy(&_cond);
	pthread_mutex_destroy(&_lock);
}

/**
 * \brief launch as many tasks as possible
 */
void	TaskQueue::launch() {
	// XXX implementation missing
}

/**
 * \brief signal that a queue id has completed
 */
void	TaskQueue::hasended(int queueid) {
	pthread_mutex_lock(&_lock);
	_idqueue.push(queueid);
	pthread_cond_signal(&_cond);
	pthread_mutex_unlock(&_lock);
}

/**
 * \brief update the task queue with the state of the entry
 */
void	TaskQueue::update(TaskQueueEntry& entry) {
	// XXX 
}

/**
 * \brief cleanup for the queueid
 */
void	TaskQueue::cleanup(int queueid) {
	// find the TaskExectuor entry
	executormap::iterator	i = executors.find(queueid);
	if (i == executors.end()) {
		return;
	}

	// make sure the database is updated
	TaskExecutorPtr	executor = i->second;
	update(executor->task());

	// remove the execturo from the queue
	executors.erase(i);
}

} // namespace task
} // namespace astro
