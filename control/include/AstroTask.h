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
#include <AstroCallback.h>
#include <mutex>
#include <condition_variable>

namespace astro {
namespace task {

typedef long taskid_t;

/**
 * \brief TaskParameters object
 *
 * The TaskParameters object contains all information needed to
 * start a task. It tells what to do. In contrast, the TaskQueueEntry
 * class below contains in addition information acquired while while
 * performing the task.
 */
class TaskParameters {
private:
	astro::camera::Exposure	_exposure;
public:
	astro::camera::Exposure&	exposure() { return _exposure; }
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

	TaskParameters();
};

/**
 * \brief TaskInfo object
 *
 * The Task info object contains all the additional information collected
 * for a Task during its execution
 */
class TaskInfo {
private:
	taskid_t	_id;
public:
	taskid_t	id() const { return _id; }
	void	id(taskid_t i) { _id = i; }

public:
	typedef enum { pending, executing, failed, cancelled, complete } taskstate;
static std::string	state2string(taskstate t);
static taskstate	string2state(const std::string& s);
private:
	taskstate	_state;
	long		_lastchange;
	std::string	_cause;
public:
	taskstate	state() const { return _state; }
	void	state(taskstate state) { _state = state; }

	long	lastchange() const { return _lastchange; }
	void	lastchange(long l) { _lastchange = l; }
	void	now();

	const std::string&	cause() const { return _cause; }
	void	cause(const std::string& c) { _cause = c; }

private:
	std::string	_filename;
public:
	const std::string& filename() const { return _filename; }
	void	filename(const std::string& filename) { _filename = filename; }

private:
	astro::image::ImageRectangle	_frame;
public:
	const astro::image::ImageRectangle&	frame() const { return _frame; }
	void	frame(const astro::image::ImageRectangle& f) { _frame = f; }

	const astro::image::ImagePoint&	origin() const { return _frame.origin(); }
	void	origin(const astro::image::ImagePoint& o) { _frame.setOrigin(o); }

	const astro::image::ImageSize&	size() const { return _frame.size(); }
	void	size(const astro::image::ImageSize& s) { _frame.setSize(s); }

	TaskInfo(taskid_t id);
};

class TaskMonitorInfo;

/**
 * \brief Task Queue entry
 *
 * The TaskQueueEntry object is an extension of the Task object. It collects
 * all information needed during task processing, like identification,
 * state and the name of the image file created from the completion of
 * the task.
 */
class TaskQueueEntry : public TaskParameters, public TaskInfo {
public:
	TaskQueueEntry(taskid_t queueid, const TaskParameters& task);

	TaskParameters	parameters() const;
	TaskInfo	info() const;

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
	typedef std::map<taskid_t, TaskExecutorPtr>	executormap;
	executormap	executors;

	// thread performing the task queue management work
	pthread_t	_thread;
	// lock to protect the data structures
	std::recursive_mutex	lock;
	// condition variable to signal to the task queue thread
	std::condition_variable_any	statechange_cond;

	// condition variable and lock used for the wait operation
	std::condition_variable_any	wait_cond;

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
	static std::string	state2string(const state_type& state);
	static state_type	string2state(const std::string& s);
private:
	state_type	_state;
public:
	const state_type&	state() const { return _state; }
private:
	std::queue<taskid_t>	_idqueue;
private:
	// prevent copying
	TaskQueue(const TaskQueue& other);
	TaskQueue&	operator=(const TaskQueue& other);
public:
	TaskQueue(astro::persistence::Database database);
	~TaskQueue();

	// recover from crash by turning all executing entries into
	// failed entries
	void	recover();

	// main function
	void	main();

private:
	// start new executors. Many methods here are private, they are
	// only used internally
	void	launch();
	void	launch(const TaskQueueEntry& entry);
	// methods to update the executormap/database
	void	update(const TaskQueueEntry& entry);
	void	update(taskid_t queueid);
	void	cleanup(taskid_t queueid);
	bool	blocks(const TaskQueueEntry& entry);
public:
	void	post(taskid_t queueid);	// signal state change for queueid
	bool	running(taskid_t queueid);

	// start and stop queue processing
	void	start();		// start queue processing
	void	stop();			// stop launching new executors
	void	cancel();		// cancel all active executors
	void	wait(taskid_t queueid);	// wait for an executor to terminate
	void	wait();			// wait for alle executors to terminate
	void	shutdown();		// shutdown the queue
	void	restart(state_type newstate = stopped);		// restart the queue
	void	remove(taskid_t queueid);
	void	cancel(taskid_t queueid);

	// submit a new task entry
	taskid_t	submit(const TaskParameters& parameters);

	// information about the queue content
	taskid_t	nexecutors() const { return executors.size(); }
	TaskExecutorPtr	executor(taskid_t queueid);
	TaskQueueEntry	entry(taskid_t queueid);
	TaskInfo	info(taskid_t queueid);
	TaskParameters	parameters(taskid_t queueid);

	// retrieve a list of a queue ids for a given state
	std::list<taskid_t>	tasklist(TaskQueueEntry::taskstate state);
	bool	exists(taskid_t queueid);

	// monitoring callback called whenever a task changes state
	astro::callback::CallbackPtr	callback;
private:
	void	call(const TaskInfo& info);
	void	call(const TaskQueueEntry& entry);
};

/**
 * \brief Task Monitor information
 */
class TaskMonitorInfo {
private:
	long	_taskid;
public:
	long	taskid() const { return _taskid; }
	void	taskid(long ti) { _taskid = ti; }
private:
	TaskQueueEntry::taskstate	_state;
public:
	TaskQueueEntry::taskstate	state() const { return _state; }
	void	state(const TaskQueueEntry::taskstate& s) { _state = s; }
private:
	time_t	_when;
public:
	time_t	when() const { return _when; }
	void	when(time_t w) { _when = w; }
};

/**
 * \brief Callback structure for the monitor
 */
class TaskMonitorCallbackData : public astro::callback::CallbackData {
	TaskMonitorInfo	_info;
public:
	TaskMonitorCallbackData(const TaskMonitorInfo& info) : _info(info) { }
	const	TaskMonitorInfo&	info() const {
		return _info;
	}
};

class ExposureTask;

/**
 * \brief TaskExecutor
 */
class TaskExecutor {
	TaskQueue&	_queue;
	TaskQueueEntry	_task;
	ExposureTask	*exposuretask;
public:
	TaskQueueEntry&	task() { return _task; }
private:
	pthread_t	_thread;
	// the _lock and _cond variables are used to communicate with the 
	// executor. The conditiona variable is signaled when the thread
	// has stared running
	std::mutex	_lock;
	std::condition_variable	_cond;
public:
	void	main();
private:
	// ensure that the TaskExecutor cannot be copied
	TaskExecutor&	operator=(const TaskExecutor& other);
	TaskExecutor(const TaskExecutor& other);

public:
	TaskExecutor(TaskQueue& queue, const TaskQueueEntry& task);
	~TaskExecutor();

private:
	// for cancel and wait methos, we use another couple of mutex and
	// condition variables. Whenever there is an occasion to wait inside
	std::mutex	wait_lock;
	std::condition_variable	wait_cond;
public:
	void	cancel();
	void	wait();
	bool	wait(float t);


	bool	blocks(const TaskQueueEntry& other);
	bool	running();

	friend class TaskQueue;
};

} // namespace task
} // namespace astro

#endif /* _AstroTask_h */
