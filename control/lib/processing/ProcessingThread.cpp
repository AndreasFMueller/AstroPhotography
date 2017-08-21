/*
 * ProcessingThead.cpp -- implementation of thread
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <pthread.h>
#include <AstroUtils.h>
#include <includes.h>
#include <mutex>
#include <condition_variable>

namespace astro {
namespace process {

#if 0
//////////////////////////////////////////////////////////////////////
// Derived Implementation class for Processing Threads
//////////////////////////////////////////////////////////////////////
/**
 * \brief Implementation class hiding the thread implementation 
 */
class ProcessingThreadImpl : public ProcessingThread {
	pthread_t	thread;
	std::recursive_mutex	_lock;
	std::condition_variable_any	_cond;
	int		_fd;
public:
	ProcessingThreadImpl(ProcessingStepPtr step);
	virtual ~ProcessingThreadImpl();
	virtual void	run(int fd = -1);
	virtual void	cancel();
	virtual void	wait();
	bool	working;
	virtual bool	isrunning();
	virtual void	work();
	virtual void	started();
};

/**
 * \brief Initialize a processing thread
 */
ProcessingThreadImpl::ProcessingThreadImpl(ProcessingStepPtr step)
	: ProcessingThread(step) {
	working = false;
}

/**
 * \brief Destroy a thread
 *
 * Destroying a thread means that we first have to cancel a running thread
 * and then wait for the thread to finish
 */
ProcessingThreadImpl::~ProcessingThreadImpl() {
	if (isrunning()) {
		try {
			cancel();
		} catch (...) { }
		try {
			wait();
		} catch (...) { }
	}
}

/**
 * \brief handler function to reset the working flag
 */
static void	cleanup_processing_thread(void *arg) {
	if (NULL == arg) {
		return;
	}
	ProcessingThreadImpl	*t = (ProcessingThreadImpl *)arg;
	t->working = false;
}

/**
 * \brief trampoline function to run the work function of a processing step
 */
static void	*run_processing_thread(void *arg) {
	// check that we have a good argument
	if (arg == NULL) {
		return NULL;
	}
	ProcessingThreadImpl	*t = (ProcessingThreadImpl *)arg;

	// push a cleanup handler that will be run when the thread terminates
	pthread_cleanup_push(cleanup_processing_thread, arg);

	// now to the step's work
	t->work();

	// cleanup, and if the cleanup was not called yet, execute it
	pthread_cleanup_pop(1);

	// return the initial argument to indicate that we were successful
	return arg;
}

/**
 * \brief Start a thread
 *
 * This method starts a thread
 */
void	ProcessingThreadImpl::run(int fd) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting thread");
	std::unique_lock<std::recursive_mutex>	lock(_lock);
	if (working) {
		throw std::runtime_error("thread already running");
	}
	_fd = fd;
	working = true;
	// create a new thread
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	if (pthread_create(&thread, &attr, run_processing_thread, this)) {
		lock.unlock();
		working = false;
		return;
	}

	// wait until the thread is in state working
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for thread to start up");
	_cond.wait(lock);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread successfully started");
}

/**
 * \brief Signal a thread to stop
 *
 * Tells a thread to stop. Not all processing steps will be capable of being
 * cancelled.
 */
void	ProcessingThreadImpl::cancel() {
	_step->cancel();
}

/**
 * \brief Wait for a thread to complete
 *
 * This method tries to join a thread. It returns as soon as it has decided
 * that the thread is no longer running. It may throw an exception for some
 * other error states.
 */
void	ProcessingThreadImpl::wait() {
	void	*result = NULL;
	if (pthread_join(thread, &result)) {
		if (errno == ESRCH) {
			// thread has already terminated
			return;
		}
		throw std::runtime_error("internal error while waiting "
			"for thread");
	}
	if (result == NULL) {
		// this means the process failed 
	}
}

/**
 * \brief Ask whether the thread is still running
 */
bool	ProcessingThreadImpl::isrunning() {
	std::unique_lock<std::recursive_mutex>	lock(_lock);
	return working;
}

/**
 * \brief signal that the thread has started
 */
void	ProcessingThreadImpl::started() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "signal that thread has started");
	{
		std::unique_lock<std::recursive_mutex>	lock(_lock);
		_cond.notify_all();
	}
}

/**
 * \brief Work function of the thread
 */
void	ProcessingThreadImpl::work() {
	try {
		_step->work(this);
	} catch (...) {

	}
	if (_fd >= 0) {
		unsigned char	rc = _step->status();
		if (1 != ::write(_fd, &rc, 1)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"cannot signal the controller: %s",
				strerror(errno));
		}
		close(_fd);
	}
}

//////////////////////////////////////////////////////////////////////
// Some methods of the ProcessingThread that were not inlined
//////////////////////////////////////////////////////////////////////
/**
 * \brief factory method of the ProcessingThread class
 */
ProcessingThreadPtr	ProcessingThread::get(ProcessingStepPtr step) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create new thread object");
	return ProcessingThreadPtr(new ProcessingThreadImpl(step));
}

/**
 * \brief Destructor for ProcessingThread
 */
// we place this here (and not in the header) to ensure that ProcessingThread
// really gets a vtable
ProcessingThread::~ProcessingThread() {
}
#endif

void	start_work(ProcessingThread *t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread start");
	t->work();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread complete");
}

ProcessingThread::ProcessingThread(ProcessingStepPtr step)
	: std::thread(start_work, this), _step(step) {
	detach();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "barrier comming up");
	sleep(1);
	_step->_barrier.await();
}

void	ProcessingThread::work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ProcessingThread::work() start");
	_step->work();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ProcessingThread::work() end");
}

ProcessingThread::~ProcessingThread() {
	//join();
}

} // namespace process
} // namespace astro
