/*
 * TaskQueue.cpp -- queue of tasks
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <unistd.h>
#include <pthread.h>
#include <TaskTable.h>

using namespace astro::persistence;

namespace astro {
namespace task {

/**
 * \brief Mutex lock class
 *
 * We want to make sure a lock is always released when it goes out of scope.
 * The distructor of the TaskQueueLock class does precisely this.
 */
class TaskQueueLock {
	pthread_mutex_t	*_lock;
public:
	TaskQueueLock(pthread_mutex_t *lock) : _lock(lock) {
		pthread_mutex_lock(_lock);
//		debug(LOG_DEBUG, DEBUG_LOG, 0, "lock %p acquired", _lock);
	}
	~TaskQueueLock() {
		pthread_mutex_unlock(_lock);
//		debug(LOG_DEBUG, DEBUG_LOG, 0, "lock %p released", _lock);
	}
};

/**
 * \brief static main function for the task queue
 *
 * This function is only used as a springboard to jump to the main function
 * in the TaskQueue object.
 */
static void	*queuemain(void *p) {
	TaskQueue	*queue = (TaskQueue *)p;
	queue->main();
	return queue;
}

/**
 * \brief  main function for the task queue
 *
 * This is the "real" main function for a task queue
 */
void	TaskQueue::main() {
	TaskQueueLock	l(&lock);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task queue thread begins executing");
	while (state() != idle) {
		// wait for the next signal, this also unlocks the lock
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"waiting for state change signal, release %p", &lock);
		if (pthread_cond_wait(&statechange_cond, &lock)) {
			throw std::runtime_error("pthread_cond_wait failed");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"statechange signal received, acquire %p", &lock);

		// process any tasks that have completed
		int	terminationcounter = 0;
		while (!_idqueue.empty()) {
			int	queueid = _idqueue.front();
			_idqueue.pop();
			cleanup(queueid);
			terminationcounter++;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d executors cleaned up",
			terminationcounter);
		// if we have terminated any executors, then some threads
		// may be waiting for them. We signal these waiters so they
		// can check whether "their" queueid has terminated
		if (terminationcounter) {
			pthread_cond_broadcast(&wait_cond);
		}

		// process any state changes
		if (state() == stopping) {
			if (0 == executors.size()) {
				_state = stopped;
				pthread_cond_broadcast(&wait_cond);
			}
		}

		// launch as many tasks as possible, this will be a noop
		// unless the queue is in the launching state
		launch();

		// during this block, the lock was held by the work thread,
		// so no other thread could do anything. In particular, the
		// cleanup waiters could not respond to the signal we sent.
		// when we restart the loop, and return to the call to
		// pthread_cond_wait, the lock is released and all these
		// waiters will so the states that have changed while they
		// were waiting for the lock
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main method ends");
}

/**
 * \brief convert state value into a string
 */
std::string	TaskQueue::statestring(const state_type& state) {
	switch (state) {
	case idle:
		return std::string("idle");
	case launching:
		return std::string("launching");
	case stopping:
		return std::string("stopping");
	case stopped:
		return std::string("stopped");
	}
}

/**
 * \brief start the worker thread
 *
 * This is only possible in the idle state, the method will throw an exception
 * otherwise.
 */
void	TaskQueue::restart() {
	TaskQueueLock	l(&lock);
	if (state() != idle) {
		throw std::runtime_error("can start thread only in idle state");
	}
	// launch the work thread
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_create(&_thread, &attr, queuemain, this);
	_state = launching;
	// at this point, the lock object goes out of scope and releases
	// the thread that was started previously
}

/**
 * \brief kill the worker thread
 */
void	TaskQueue::shutdown() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown requested");
	{
		TaskQueueLock	l(&lock);
		if (state() != stopped) {
			throw std::runtime_error("can shutdown only when stopped");
		}

		// inform the work thread that it should terminate
		_state = idle;
		pthread_cond_signal(&statechange_cond);

		// at this point, the lock goes out of scope and releases the
		// work thread. So the only thing we have to do after this
		// point is to wait for the thread to finish
	}

	// wait for the thread to terminate
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for thread to terminated");
	pthread_join(_thread, NULL);
}

/**
 * \brief create a task queue
 *
 * The constructor does not also start the queue. For this purpose, the
 * method start() should be called, which then 
 */
TaskQueue::TaskQueue(Database database) : _database(database) {
	// initialize state variables
	_state = idle;
	
	// initialize the mutex lock
	pthread_mutexattr_t	mattr;
        pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&lock, &mattr);

	// initialize the condition variables
	pthread_condattr_t	cattr;
	pthread_condattr_init(&cattr);
	pthread_cond_init(&statechange_cond, &cattr);	// state change
	pthread_cond_init(&wait_cond, &cattr);		// waiting

	// launch the thread
	restart();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task queue thread launched");
}

/**
 * \brief Stop the task queue
 *
 * The destructor stops the thread, and destroys mutex lock and condition
 * variable
 */
TaskQueue::~TaskQueue() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying task queue");
	// ensure we are the only ones accessing the data structures
	pthread_mutex_lock(&lock);

	// stop everything
	stop();

	// cancel all currently running executors
	cancel();

	// wait for all executors to stop
	wait();

	// shutdown the work thread
	shutdown();

	// now destroy the pthread resources
	pthread_cond_destroy(&statechange_cond);
	pthread_cond_destroy(&wait_cond);
	pthread_mutex_unlock(&lock);
	pthread_mutex_destroy(&lock);
}

/**
 * \brief Check whether any executor blocks a given new task
 */
bool	TaskQueue::blocks(const TaskQueueEntry& entry) {
	executormap::iterator	i;
	for (i = executors.begin(); i != executors.end(); i++) {
		if (i->second->blocks(entry)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d blocks %d",
				i->first, entry.id());
			return true;
		}
	}
	return false;
}

/**
 * \brief launch a specific executors
 *
 * This method creates a copy of the entry, starts an executor on it, and
 * places the executor in the executor map.
 */
void	TaskQueue::launch(const TaskQueueEntry& entry) {
	// create a a task executor from the TaskQueueEntry, this also
	// starts the work thread, but blocks it because the first thing
	// it wants to do in its work thread is to change the state
	TaskExecutorPtr	_executor(new TaskExecutor(*this, entry));

	// add the task entry to the map
	executormap::value_type	mapentry(entry.id(), _executor);
	executors.insert(mapentry);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "new entry %d added to executors map",
		entry.id());
}

/**
 * \brief launch as many tasks as possible
 */
void	TaskQueue::launch() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launching all possible pending task");
	// private method, only called from methods that have already locked
	// the queue
	TaskTable	tasktable(_database);
	std::list<long>	idlist = tasktable.selectids("state = 0 order by id");

	// go through the list of ids
	std::list<long>::const_iterator	i;
	for (i = idlist.begin(); i != idlist.end(); i++) {
		long	id = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "checking id %d", id);
		TaskQueueEntry	entry = tasktable.byid(id);
		if (!blocks(entry)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "launching %d", id);
			launch(entry);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "id %d is blocked", id);
		}
	}
}

/**
 * \brief submit a new entry
 *
 * This is currently only a test function, it bypasses the launch method
 * completely, and does not involve the database. Later, this will be modified
 * so that new tasks are written to the database, and launched from the
 * launch method.
 */
int	TaskQueue::submit(const Task& task) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit new task");
	TaskQueueLock	l(&lock);

	// add the entry to the task table
	TaskTable	tasktable(_database);
	TaskQueueEntry	entry(0, task);
	entry.state(TaskQueueEntry::pending);
	long taskqueueid = tasktable.add(entry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task with id %d added to table",
		taskqueueid);

	// call launch, which will lauch all entries 
	launch();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submitted new queueid %d", taskqueueid);
	return taskqueueid;
}

/**
 * \brief signal that a queue id has changed.
 *
 * This method has to find out what happend with the task. First it updates
 * the task in the database. If the task has completed, either by failing
 * or by returning an image, then it has to be cleaned up, so we add an entry
 * to the cleanup queue and signal the work thread that it can do the cleanup.
 */
void	TaskQueue::post(int queueid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "post an update for id %d", queueid);
	TaskQueueLock	l(&lock);
	// get the entry, and update the database
	TaskExecutorPtr	e = executor(queueid);
	update(e->task());

	// now check whether cleanup is necessary
	if ((e->task().state() == TaskQueueEntry::failed) ||
		(e->task().state() == TaskQueueEntry::cancelled) ||
		(e->task().state() == TaskQueueEntry::complete)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "signal task completion");
		_idqueue.push(queueid);
		pthread_cond_signal(&statechange_cond);
	}
	// when the lock goes out of scope, the work thread will wake up and
	// will start processing the state change
}

/**
 * \brief update the task queue with the state of the entry
 */
void	TaskQueue::update(const TaskQueueEntry& entry) {
	TaskQueueLock	l(&lock);
	// send update to database
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update the task table");
	TaskTable	tasktable(_database);
	tasktable.update(entry.id(), entry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update entry %d in database, state %s",
		entry.id(), statestring(state()).c_str());
}

/**
 * \brief cancel an executor
 */
void	TaskQueue::cancel(int queueid) {
	executormap::iterator	emapp = executors.find(queueid);
	if (executors.end() == emapp) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelling queueid %d", queueid);
	emapp->second->cancel();
	// the cancel call blocks until the update has been posted to
	// the queue
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cleaning up queueid %d", queueid);

	// wait for termination of the the executor
	TaskExecutorPtr	executor = i->second;
	executor->wait();

	// remove the execturo from the queue
	executors.erase(i);
}

/**
 * \brief start queue processing
 */
void	TaskQueue::start() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the queue");
	// start processing the queue
	TaskQueueLock	l(&lock);
	_state = launching;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start launching executors");
	launch();
}

/**
 * \brief stop launching new processes
 *
 * This causes the TaskQueue to no longer launch any new executors. It can
 * still take quite some time until all active executors have terminated.
 */
void	TaskQueue::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop the queue");
	TaskQueueLock	l(&lock);
	// stop launching new executors
	if (nexecutors() == 0) {
		_state = stopped;
	} else {
		_state = stopping;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no longer launching new executors");
}

/**
 * \brief cancel all active executors
 *
 * This sends all active executors the cancel signal. This alone does not
 * prevent new executors from being launched. So to get the TaskQueue idle,
 * one has to first stop it using the stop() method, then cancel all the
 * remaining active executors. Alternatively, one can use the wait method
 * to wait for the executors to terminate.
 */
void	TaskQueue::cancel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel all executors");
	TaskQueueLock	l(&lock);
	executormap::iterator	emapp;
	for (emapp = executors.begin(); emapp != executors.end(); emapp++) {
		cancel(emapp->first);
	}
}

/**
 * \brief Wait for a specific executor to terminate
 */
void	TaskQueue::wait(int queueid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for queueid %d", queueid);
	TaskQueueLock	l(&lock);
	while (running(queueid)) {
		pthread_cond_wait(&wait_cond, &lock);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait signal received");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "queueid %d not (any longer) executing",
		queueid);
}

/**
 * \brief Wait for all executors to terminate
 *
 * This method waits for all active executors to terminate. The method
 * only does something when the queue is in the stopping state. In the
 * idle or launching state, it does not make sense to wait, the method
 * throws an exception.
 */
void	TaskQueue::wait() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for all executors");
	TaskQueueLock	l(&lock);
	if ((state() == idle) || (state() == launching)) {
		throw std::runtime_error("cannot wait in idle/launching state");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "queue is %sstopping",
		(state() == stopping) ? "" : "NOT ");
	while (state() != stopped) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for stopped state");
		pthread_cond_wait(&wait_cond, &lock);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no executors executing (any longer)");
}

/**
 * \brief Find a TaskQueueEntry
 */
TaskExecutorPtr	TaskQueue::executor(int queueid) {
	executormap::iterator	i = executors.find(queueid);
	if (executors.end() == i) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no executor with id %d", queueid);
		throw std::runtime_error("no executor with that id");
	}
	return i->second;
}

/**
 * \brief Find out whether a queue id is running
 */
bool	TaskQueue::running(int queueid) {
	executormap::iterator	i = executors.find(queueid);
	return (executors.end() != i);
	
}

} // namespace task
} // namespace astro
