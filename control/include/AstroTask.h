/*
 * AstroTask.h -- task objects, contain all the information for a task
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTask_h
#define _AstroTask_h

#include <AstroCamera.h>
#include <queue>
#include <pthread.h>
#include <AstroPersistence.h>

namespace astro {
namespace task {

/**
 * \brief Task object
 *
 * The Task object contains all information needed to start a task. It tells
 * what to do. In contrast, the TaskQueueEntry class below contains in
 * addition information acquired while while performing the task.
 */
class Task {
private:
	astro::camera::Exposure	_exposure;
public:
	const astro::camera::Exposure&	exposure() const { return _exposure; }
	void	exposure(const astro::camera::Exposure& exposure) {
		_exposure = exposure;
	}

private:
	std::string	_camera;
public:
	const std::string&	camera() const { return _camera; }
	void	camera(const std::string& camera) { _camera = camera; }

private:
	long	_ccdid;
public:
	long	ccdid() const { return _ccdid; }
	void	ccdid(long ccdid) { _ccdid = ccdid; }

private:
	float	_ccdtemperature;
public:
	float	ccdtemperature() const { return _ccdtemperature; }
	void	ccdtemperature(float ccdtemperature) {
			_ccdtemperature = ccdtemperature;
	}

private:
	std::string	_filterwheel;
public:
	const std::string&	filterwheel() const { return _filterwheel; }
	void	filterwheel(const std::string& filterwheel) {
			_filterwheel = filterwheel;
	}

private:
	long	_filterposition;
public:
	long	filterposition() const { return _filterposition; }
	void	filterposition(long filterposition) {
			_filterposition = filterposition;
	}

	Task();
};

/**
 * \brief Task Queue entry
 *
 * The TaskQueueEntry object is an extension of the Task object. It collects
 * all information needed during task processing, like identification,
 * state and the name of the image file created from the completion of
 * the task.
 */
class TaskQueueEntry : public Task {
private:
	long	_id;
public:
	const long	id() const { return _id; }
	void	id(long i) { _id = i; }

public:
	typedef enum { pending, executing, failed, cancelled, complete } taskstate;
private:
	taskstate	_state;
public:
	taskstate	state() const { return _state; }
	void	state(taskstate state) { _state = state; }

private:
	std::string	_filename;
public:
	const std::string& filename() const { return _filename; }
	void	filename(const std::string& filename) { _filename = filename; }

	TaskQueueEntry(long queueid, const Task& task);
public:

	// find out whether a this task blocks some other task
	bool	blocks(const TaskQueueEntry& other) const;
	bool	blockedby(const TaskQueueEntry& other) const;
};
typedef std::shared_ptr<TaskQueueEntry>	TaskQueueEntryPtr;

class	TaskExecutor;
typedef std::shared_ptr<TaskExecutor>	TaskExecutorPtr;

/**
 * \brief Task queue object
 *
 * The TaskQueue object manages a queue of tasks. Each task is launched
 * with an executor object, and the queue can wait for completion of the
 * task.
 */
class TaskQueue {
	// database containing the task queue table
	astro::persistence::Database	_database;

	// a map containing all active executors
	typedef std::map<int, TaskExecutorPtr>	executormap;
	executormap	executors;

	// thread performing the task queue management work
	pthread_t	_thread;
	// lock to protect the data structures
	pthread_mutex_t	lock;
	// condition variable to signal to the task queue thread
	pthread_cond_t	statechange_cond;

	// condition variable and lock used for the wait operation
	pthread_cond_t	wait_cond;

	// various variables used to exchange information with 
public:
	// the task queue implements the following state diagram
	//
	//      +------+                              +-----------+
	// ---> | idle | -----start_work_thread-----> | launching |
	//      +------+        [ restart() ]         +-----------+
	//         ^                              ^    |         ^
	//         |                            /      |         |
	//         |                          /        |         |
	//     shutdown()           start()         stop()    start()
	//         |            /                      |         |
	//         |          /                        |         |
	//         |        /                          v         |
	//    +---------+ /                           +----------+
	//    | stopped | <---last_executor_stops --- | stopping |
	//    +---------+         [ wait() ]          +----------+
	//
	// In the idle state, there is no work available, so the task
	// object cannot do any work. In the launching state, the task
	// queue will start new task executors when another task 
	// completes. To prevent this, one has to stop the queue using
	// the stop() method. It will continue monitoring the active
	// executors, but it will no longer launch new executors. Then
	// the last executor has stopped, the queue goes into the stopped
	// state. From there, launching of executors can still be resumed
	// by calling the start method.
	typedef enum { idle, launching, stopping, stopped } state_type;
	static std::string	statestring(const state_type& state);
private:
	state_type	_state;
public:
	const state_type&	state() const { return _state; }
private:
	std::queue<int>	_idqueue;
private:
	// prevent copying
	TaskQueue(const TaskQueue& other);
	TaskQueue&	operator=(const TaskQueue& other);
public:
	TaskQueue(astro::persistence::Database database);
	~TaskQueue();

	// main function
	void	main();

private:
	// start new executors. Many methods here are private, they are
	// only used internally
	void	launch();
	void	launch(const TaskQueueEntry& entry);
	// methods to update the executormap/database
	void	update(const TaskQueueEntry& entry);
	void	update(int queueid);
	void	cancel(int queueid);
	void	cleanup(int queueid);
	bool	blocks(const TaskQueueEntry& entry);
public:
	void	post(int queueid);	// signal state change for queueid
	bool	running(int queueid);

	// start and stop queue processing
	void	start();		// start queue processing
	void	stop();			// stop launching new executors
	void	cancel();		// cancel all active executors
	void	wait(int queueid);	// wait for an executor to terminate
	void	wait();			// wait for alle executors to terminate
	void	shutdown();		// shutdown the queue
	void	restart();		// restart the queue

	// submit a new task entry
	int	submit(const Task& entry);

	// information about the queue content
	int	nexecutors() const { return executors.size(); }
	TaskExecutorPtr	executor(int queueid);
};

/**
 * \brief TaskExecutor
 */
class TaskExecutor {
	TaskQueue&	_queue;
	TaskQueueEntry	_task;
public:
	const TaskQueueEntry&	task() { return _task; }
private:
	pthread_t	_thread;
	pthread_mutex_t	_lock;
	pthread_cond_t	_cond;
private:
	// ensure that the TaskExecutor cannot be copied
	TaskExecutor&	operator=(const TaskExecutor& other);
	TaskExecutor(const TaskExecutor& other);

	// private timed wait function
	bool	wait(float t);
public:
	TaskExecutor(TaskQueue& queue, const TaskQueueEntry& task);
	~TaskExecutor();

	void	cancel();
	void	wait();

	void	main();

	bool	blocks(const TaskQueueEntry& other);

	friend class TaskQueue;
};

} // namespace task
} // namespace astro

#endif /* _AstroTask_h */
