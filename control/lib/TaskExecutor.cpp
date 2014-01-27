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
#include <AstroLoader.h>
#include <AstroDevaccess.h>
#include <ImageDirectory.h>
#include <errno.h>
#include <string.h>

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
 * \brief Exposure Task class
 *
 * This class encapsulates what the executor has to do
 */
class ExposureTask {
	astro::camera::CameraPtr	camera;
	astro::camera::CcdPtr		ccd;
	astro::camera::CoolerPtr	cooler;
	astro::camera::FilterWheelPtr	filterwheel;
	TaskExecutor&	_executor;
	TaskQueueEntry&	_task;
public:
	ExposureTask(TaskExecutor& executor, TaskQueueEntry& task);
	~ExposureTask();
	void	run();
};

ExposureTask::ExposureTask(TaskExecutor& executor, TaskQueueEntry& task)
	: _executor(executor), _task(task) {
	// create a repository, we are always using the default
	// repository
	astro::module::Repository	repository;
	
	// get camera and ccd
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera '%s' and ccd %d",
		_task.camera().c_str(), _task.ccdid());
	astro::device::DeviceAccessor<astro::camera::CameraPtr>
		dc(repository);
	camera = dc.get(_task.camera());
	ccd = camera->getCcd(_task.ccdid());

	// turn on the cooler
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set cooler to %f",
		_task.ccdtemperature());
	if ((ccd->hasCooler()) && (_task.ccdtemperature() > 0)) {
		cooler = ccd->getCooler();
	}

	// get the filterwheel
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get filter %d of wheel '%s'",
		_task.filterposition(), _task.filterwheel().c_str());
	astro::device::DeviceAccessor<astro::camera::FilterWheelPtr>
		df(repository);
	if ((_task.filterwheel().size() > 0) && (_task.filterposition() >= 0)) {
		filterwheel = df.get(_task.filterwheel());
	}
}

void	ExposureTask::run() {
	// set the cooler
	if (cooler) {
		cooler->setTemperature(_task.ccdtemperature());
		cooler->setOn(true);
	}

	// set the filterwheel position
	std::string	filtername("NONE");
	if (filterwheel) {
		// XXX make sure filter wheel is ready, but this is a
		//     waste of time
		filterwheel->wait(10);
		filterwheel->select(_task.filterposition());
		filtername = filterwheel->filterName(_task.filterposition());
	}

	// wait for the cooler, if present, but at most 30 seconds
	if (cooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for cooler");
		if (!cooler->wait(30)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"cannot stabilize temperature");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler now stable");
	}

	// wait for the filterwheel if present
	if (filterwheel) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for filterwheel");
		if (!filterwheel->wait(30)) {
			throw std::runtime_error("filter wheel does not idle");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel now idle");
	}

	// start exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start exposure");
	ccd->startExposure(_task.exposure());

	// wait for completion of exposure
	if (_executor.wait(_task.exposure().exposuretime)) {
		// wait for the ccd to complete
		if (ccd->wait()) {
			// get the image from the ccd
			astro::image::ImagePtr	image = ccd->getImage();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image frame: %s",
				image->getFrame().toString().c_str());

			// add filter information to the image, if present
			image->setMetadata("FILTER", Metavalue(filtername, ""));

			// add to the ImageDirectory
			astro::image::ImageDatabaseDirectory	imagedir;
			std::string	filename = imagedir.save(image);

			// remember the filename
			_task.filename(filename);

			// update the frame information
			astro::camera::Exposure	exposure = _task.exposure();
			_task.exposure(exposure);

			// copy the returned image information
			_task.size(image->size());
			_task.origin(image->origin());

			// log info
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"file %s written",
				_task.filename().c_str());
			_task.state(TaskQueueEntry::complete);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"could not get image");
			_task.state(TaskQueueEntry::failed);
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread cancelled");
		_task.state(TaskQueueEntry::cancelled);
	}
}

ExposureTask::~ExposureTask() {
	// turn of the cooler
	if (cooler) {
		cooler->setOn(false);
	}
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

		ExposureTask	exposuretask(*this, _task);

		exposuretask.run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "executor failure: %s", x.what());
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
