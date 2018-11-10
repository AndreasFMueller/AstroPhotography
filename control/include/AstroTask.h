/*
 * AstroTask.h -- task objects, contain all the information for a task
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTask_h
#define _AstroTask_h

#include <AstroCamera.h>
#include <queue>
#include <AstroPersistence.h>
#include <AstroCallback.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <AstroUtils.h>

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
	int	_cameraindex;
	int	_ccdindex;
	int	_coolerindex;
	int	_filterwheelindex;
	int	_mountindex;
	int	_focuserindex;
public:
	int	cameraindex() const { return _cameraindex; }
	void	cameraindex(int i) { _cameraindex = i; }
	int	ccdindex() const { return _ccdindex; }
	void	ccdindex(int i) { _ccdindex = i; }
	int	coolerindex() const { return _coolerindex; }
	void	coolerindex(int i) { _coolerindex = i; }
	int	filterwheelindex() const { return _filterwheelindex; }
	void	filterwheelindex(int i) { _filterwheelindex = i; }
	int	mountindex() const { return _mountindex; }
	void	mountindex(int i) { _mountindex = i; }
	int	focuserindex() const { return _focuserindex; }
	void	focuserindex(int i) { _focuserindex = i; }

private:
	std::string	_instrument;
public:
	const std::string&	instrument() const { return _instrument; }
	void	instrument(const std::string& instrument) {
		_instrument = instrument;
	}

private:
	float	_ccdtemperature;
public:
	float	ccdtemperature() const { return _ccdtemperature; }
	void	ccdtemperature(float ccdtemperature) {
			_ccdtemperature = ccdtemperature;
	}

private:
	std::string	_filter;
public:
	const std::string&	filter() const { return _filter; }
	void	filter(const std::string filter) { _filter = filter; }

private:
	std::string	_project;
public:
	const std::string&	project() const { return _project; }
	void	project(const std::string& p) { _project = p; }

private:
	std::string	_repodb;
public:
	const std::string&	repodb() const { return _repodb; }
	void	repodb(const std::string& r) { _repodb = r; }

private:
	std::string	_repository;
public:
	const std::string&	repository() const { return _repository; }
	void	repository(const std::string& r) { _repository = r; }

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
	std::string	_camera;
	std::string	_ccd;
	std::string	_cooler;
	std::string	_filterwheel;
	std::string	_mount;
	std::string	_focuser;
public:
	const std::string&	camera() const { return _camera; }
	void	camera(const std::string& camera) { _camera = camera; }
	const std::string&	ccd() const { return _ccd; }
	void	ccd(const std::string& ccd) { _ccd = ccd; }
	const std::string&	cooler() const { return _cooler; }
	void	cooler(const std::string& cooler) { _cooler = cooler; }
	const std::string&	filterwheel() const { return _filterwheel; }
	void	filterwheel(const std::string& filterwheel) {
			_filterwheel = filterwheel;
	}
	const std::string&	mount() const { return _mount; }
	void	mount(const std::string& mount) { _mount = mount; }
	const std::string&	focuser() const { return _focuser; }
	void	focuser(const std::string& focuser) { _focuser = focuser; }

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

	std::string	toString() const;
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
	std::thread	_thread;
	// lock to protect the data structures
	std::recursive_mutex	queue_mutex;
	// condition variable to signal to the task queue thread
	std::condition_variable_any	statechange_cond;

	// condition variable and lock used for the wait operation
	// task executors that have completed use this condition variable
	// to signal their state change to the thread that processes the
	// queue
	std::condition_variable_any	wait_cond;

	// various variables used to exchange information with 
public:
	// the task queue implements the following state diagram
	//
	//      +------+           start()            +-----------+
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

public:
	// public interface to change the state of the queue
	void	start();		// start queue processing
	void	stop();			// stop launching new executors
	void	shutdown();		// shutdown the queue
	void	wait();			// wait for alle executors to terminate
	void	cancel();		// cancel all active executors

private:
	// the idqueue contains the task ids of the task executors that
	// need a status update
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
	thread::Barrier	_barrier;

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

private:
	void	post(taskid_t queueid);	// signal state change for queueid
	friend class TaskExecutor;	// this allows the task executor to post
					// state changes to the queue
	bool	running(taskid_t queueid);

	// private interface to the task queue
	void	wait(taskid_t queueid);	// wait for an executor to terminate
	void	restart(state_type newstate = stopped);	 // restart the queue

public:
	void	remove(taskid_t queueid);
	void	cancel(taskid_t queueid);

	// submit a new task entry
	taskid_t	submit(const TaskParameters& parameters,
				const TaskInfo& info);

	// information about the queue content
	taskid_t	nexecutors() const { return executors.size(); }
private:
	TaskExecutorPtr	executor(taskid_t queueid);
	TaskQueueEntry	entry(taskid_t queueid);
public:
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
	TaskQueueEntry::taskstate	_oldstate;
public:
	TaskQueueEntry::taskstate	oldstate() const { return _oldstate; }
	void	oldstate(const TaskQueueEntry::taskstate& s) { _oldstate = s; }
private:
	time_t	_when;
public:
	time_t	when() const { return _when; }
	void	when(time_t w) { _when = w; }
};

/**
 * \brief Callback structure for the monitor
 */
typedef callback::CallbackDataEnvelope<TaskMonitorInfo>	TaskMonitorCallbackData;

class CancellableWork;

/**
 * \brief TaskExecutor
 *
 * The task executor holds the thread that performs the actual work. The work
 * is divided among the main() method and the exposurework class. The main()
 * method does the stuff related to maintaining the state in the task executor,
 * and the ExposreWork class does the exposure specific stuff.
 */
class TaskExecutor {
	TaskQueue&	_queue;
	TaskQueueEntry	_task;
	CancellableWork	*exposurework;
public:
	TaskQueueEntry&	task() { return _task; }
private:
	std::thread	_thread;
public:
	void	main();
private:
	// ensure that the TaskExecutor cannot be copied
	TaskExecutor&	operator=(const TaskExecutor& other);
	TaskExecutor(const TaskExecutor& other);

public:
	TaskExecutor(TaskQueue& queue, const TaskQueueEntry& task);
	void	release();
	~TaskExecutor();

private:
	// the release_mutex is used to block the thread from doing
	// to much work, in particular from trying to post it's state 
	// to the task queue before the task queue is ready to receive
	// it. The constructor initially locks the mutex, and the 
	thread::Barrier	_barrier;
public:
	void	cancel();
	void	wait();

	bool	blocks(const TaskQueueEntry& other);
	bool	running();

	friend class TaskQueue;
};

} // namespace task
} // namespace astro

#endif /* _AstroTask_h */
