/*
 * ProcessingThead.cpp -- implementation of thread
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <pthread.h>
#include <AstroUtils.h>

namespace astro {
namespace process {

/**
 * \brief Implementation class hiding the thread implementation 
 */
class ProcessingThreadImpl : public ProcessingThread {
	pthread_t	thread;
	pthread_mutex_t	_lock;
public:
	ProcessingThreadImpl(ProcessingStepPtr step);
	virtual ~ProcessingThreadImpl();
	virtual void	run();
	virtual void	cancel();
	virtual void	wait();
	bool	working;
	virtual bool	isrunning();
	virtual void	work();
};

/**
 * \brief Initialize a processing thread
 */
ProcessingThreadImpl::ProcessingThreadImpl(ProcessingStepPtr step)
	: ProcessingThread(step) {
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_lock, &mattr);
	working = false;
}

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
void	ProcessingThreadImpl::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting thread");
	PthreadLocker	lock(&_lock);
	if (working) {
		throw std::runtime_error("thread already running");
	}
	working = true;
	// create a new thread
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	if (pthread_create(&thread, &attr, run_processing_thread, this)) {
		working = false;
		return;
	}
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
	PthreadLocker	lock(&_lock);
	return working;
}

/**
 * \brief Work function of the thread
 */
void	ProcessingThreadImpl::work() {
	try {
		_step->work();
	} catch (...) {

	}
}

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

} // namespace process
} // namespace astro
