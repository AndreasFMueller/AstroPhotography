/*
 * TaskQueue.cpp -- queue of tasks
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <unistd.h>
#include <pthread.h>
#include <TaskTable.h>

using namespace astro::persistence;

namespace astro {
namespace task {

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
	std::unique_lock<std::recursive_mutex>	l(lock);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task queue thread begins executing");
	while (state() != idle) {
		// wait for the next signal, this also unlocks the lock
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"waiting for state change signal, release %p", &lock);
		statechange_cond.wait(l);
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
			wait_cond.notify_all();
		}

		// process any state changes
		if (state() == stopping) {
			if (0 == executors.size()) {
				_state = stopped;
				wait_cond.notify_all();
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
std::string	TaskQueue::state2string(const state_type& state) {
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
	throw std::runtime_error("invalid state");
}

/**
 * \brief convert state string into a state value
 */
TaskQueue::state_type	TaskQueue::string2state(const std::string& s) {
	if (s == "idle") {
		return idle;
	}
	if (s == "launching") {
		return launching;
	}
	if (s == "stopping") {
		return stopping;
	}
	if (s == "stopped") {
		return stopped;
	}
	throw std::runtime_error("invalid state name");
}

/**
 * \brief start the worker thread
 *
 * This is only possible in the idle state, the method will throw an exception
 * otherwise.
 */
void	TaskQueue::restart(state_type newstate) {
	std::unique_lock<std::recursive_mutex>	l(lock);
	if (state() != idle) {
		throw std::runtime_error("can start thread only in idle state");
	}
	if (newstate == idle) {
		throw std::runtime_error("cannot restart into idle state");
	}
	// launch the work thread
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	pthread_create(&_thread, &attr, queuemain, this);
	_state = newstate;
	// at this point, the lock object goes out of scope and releases
	// the thread that was started previously
}

/**
 * \brief kill the worker thread
 */
void	TaskQueue::shutdown() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown requested");
	{
		std::unique_lock<std::recursive_mutex>	l(lock);
		if (state() != stopped) {
			throw std::runtime_error("can shutdown only when stopped");
		}

		// inform the work thread that it should terminate
		_state = idle;
		statechange_cond.notify_one();

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
	std::unique_lock<std::recursive_mutex>	l(lock);

	// stop everything
	stop();

	// cancel all currently running executors
	cancel();

	// wait for all executors to stop
	wait();

	// shutdown the work thread
	shutdown();
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
	if (_state != launching) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not launching");
		return;
	}
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
 * This method creates a new entry in the database. It then calls launch,
 * which will launch all possible tasks.
 */
taskid_t	TaskQueue::submit(const TaskParameters& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit new task");
	std::unique_lock<std::recursive_mutex>	l(lock);

	// add the entry to the task table
	TaskTable	tasktable(_database);
	TaskQueueEntry	entry(0, parameters);
	entry.state(TaskQueueEntry::pending);
	entry.now();
	long taskqueueid = tasktable.add(entry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task with id %d added to table",
		taskqueueid);
	entry.id(taskqueueid);

	// inform any monitor client about the new entry
	call(entry);

	// call launch, which will lauch all entries 
	launch();

	// return the queue id
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
void	TaskQueue::post(taskid_t queueid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "post an update for id %d", queueid);
	std::unique_lock<std::recursive_mutex>	l(lock);
	// get the entry, and update the database
	TaskExecutorPtr	e = executor(queueid);
	e->task().now();
	update(e->task());

	// now check whether cleanup is necessary
	if ((e->task().state() == TaskQueueEntry::failed) ||
		(e->task().state() == TaskQueueEntry::cancelled) ||
		(e->task().state() == TaskQueueEntry::complete)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "signal task completion");
		_idqueue.push(queueid);
		statechange_cond.notify_one();
	}
	// when the lock goes out of scope, the work thread will wake up and
	// will start processing the state change
}

using astro::callback::CallbackDataPtr;

/**
 * \brief update the task queue with the state of the entry
 */
void	TaskQueue::update(const TaskQueueEntry& entry) {
	std::unique_lock<std::recursive_mutex>	l(lock);
	// send update to database
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update the task table");
	TaskTable	tasktable(_database);
	tasktable.update(entry.id(), entry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update entry %d in database, state %s",
		entry.id(), state2string(state()).c_str());

	// inform the clients
	call(entry);
}

/**
 * \brief Call the callback with the info
 */
void	TaskQueue::call(const TaskInfo& info) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "info.id() = %d", info.id());
	// if there is no callback, there is nothing to do
	if (!callback) {
		return;
	}

	// distribute the updates also to the callback
	TaskMonitorInfo	monitorinfo;
	monitorinfo.state(info.state());
	monitorinfo.taskid(info.id());
	monitorinfo.when(time(NULL));

	CallbackDataPtr	cbd(new TaskMonitorCallbackData(monitorinfo));
	(*callback)(cbd);
}

/**
 * \brief Call the callback for a taskentry
 */
void	TaskQueue::call(const TaskQueueEntry& entry) {
	call(entry.info());
}

/**
 * \brief cancel an executor
 */
void	TaskQueue::cancel(taskid_t queueid) {
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
void	TaskQueue::cleanup(taskid_t queueid) {
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
 * \brief Remove a task
 */
void	TaskQueue::remove(taskid_t queueid) {
	executormap::iterator	i = executors.find(queueid);
	if (i != executors.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "executor for %d present",
			queueid);
		throw std::runtime_error("process still executing");
	}

	// remove task from teh database
	std::unique_lock<std::recursive_mutex>	l(lock);

	// since we want later to inform the client via the callback, we have
	// to read the entry first
	TaskInfo	taskinfo = info(queueid);

	// remove the entry
	TaskTable	tasktable(_database);
	tasktable.remove(queueid);

	// if we can uccessfully remove the entry, then we should inform
	// the clients that the state has changed
	call(taskinfo);
}

/**
 * \brief start queue processing
 */
void	TaskQueue::start() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the queue");
	// start processing the queue
	std::unique_lock<std::recursive_mutex>	l(lock);
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
	std::unique_lock<std::recursive_mutex>	l(lock);
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
	std::unique_lock<std::recursive_mutex>	l(lock);
	executormap::iterator	emapp;
	for (emapp = executors.begin(); emapp != executors.end(); emapp++) {
		cancel(emapp->first);
	}
}

/**
 * \brief Wait for a specific executor to terminate
 */
void	TaskQueue::wait(taskid_t queueid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for queueid %ld", queueid);
	std::unique_lock<std::recursive_mutex>	l(lock);
	while (running(queueid)) {
		wait_cond.wait(l);
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
	std::unique_lock<std::recursive_mutex>	l(lock);
	if ((state() == idle) || (state() == launching)) {
		throw std::runtime_error("cannot wait in idle/launching state");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "queue is %sstopping",
		(state() == stopping) ? "" : "NOT ");
	while (state() != stopped) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for stopped state");
		wait_cond.wait(l);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "no executors executing (any longer)");
}

/**
 * \brief Find a TaskExecutor
 */
TaskExecutorPtr	TaskQueue::executor(taskid_t queueid) {
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
bool	TaskQueue::running(taskid_t queueid) {
	executormap::iterator	i = executors.find(queueid);
	return (executors.end() != i);
	
}

/**
 * \brief retrieve a list of tasks with a given state
 */
std::list<long>	TaskQueue::tasklist(TaskQueueEntry::taskstate state) {
	TaskTable	tasktable(_database);
	std::list<long>	idlist = tasktable.selectids(
		stringprintf("state = %d order by id", state));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d ids found", idlist.size());
	return idlist;
}

/**
 * \brief  find out whether the queue id exists
 */
bool	TaskQueue::exists(taskid_t queueid) {
	TaskTable	tasktable(_database);
	return tasktable.exists(queueid);
}

TaskQueueEntry	TaskQueue::entry(taskid_t queueid) {
	TaskTable	tasktable(_database);
	return tasktable.byid(queueid);
}

/**
 * \brief Retrieve info about a task
 */
TaskInfo        TaskQueue::info(taskid_t queueid) {
	return entry(queueid).info();
}

/**
 * \brief Retrieve parameters of a task
 */
TaskParameters  TaskQueue::parameters(taskid_t queueid) {
	return entry(queueid).parameters();
}

/**
 * \brief Recover from a crash
 */
void	TaskQueue::recover() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "database recovery");
	std::unique_lock<std::recursive_mutex>	l(lock);

	time_t	now = time(NULL);
	std::string	query = stringprintf(
		"update taskqueue set "
		"state = %d, "
		"lastchange = %d, "
		"cause = 'server crash' "
		"where state = %d", TaskInfo::failed, now,
		TaskInfo::executing);
	try {
		_database->query(query);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error in recovery query: %s",
			x.what());
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query '%s' fixed database consistency",
		query.c_str());
}

} // namespace task
} // namespace astro
