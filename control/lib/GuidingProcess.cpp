/*
 * GuidingProcess.cpp -- guiding processes base clasess
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuidingProcess.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {
namespace guiding {

//////////////////////////////////////////////////////////////////////
// ThreadBase methods
//////////////////////////////////////////////////////////////////////

class RunAccess {
	ThreadBase&	_threadbase;
public:
	RunAccess(ThreadBase& threadbase) : _threadbase(threadbase) { }
	void	main() { _threadbase.run(); }
};

/**
 * \brief generic springboard function
 */
static void	*springboard_main(void *args) {
	ThreadBase	*g = (ThreadBase *)args;
	RunAccess(*g).main();
	return args;
}

/**
 * \brief construct a ThreadBase
 */
ThreadBase::ThreadBase() {
	// mutex
	pthread_mutexattr_t	mattr;
	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mutex, &mattr);
	GuidingLock	lock(&mutex);

	// cond
	pthread_condattr_t	cattr;
	pthread_condattr_init(&cattr);
	pthread_cond_init(&waitcond, &cattr);

	// at this point the lock object goes out of scope and thus
	// unlocks the mutex
	_isrunning = false;
	_terminate = false;
}

/**
 * \brief destroy the thread
 */
ThreadBase::~ThreadBase() {
	GuidingLock	lock(&mutex);
	try {
		stop();
	} catch (...) {
	}
}

/**
 * \brief find out whether a thread is running
 */
bool	ThreadBase::isrunning() {
	GuidingLock	lock(&mutex);
	return _isrunning;
}

/**
 * \brief start the thread
 */
void	ThreadBase::start() {
	GuidingLock	lock(&mutex);

	if (isrunning()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread already running");
		throw std::runtime_error("thread already running");
	}

	_terminate = false;
	_isrunning = false;

	// start the thread
	pthread_attr_t	attr;
	pthread_attr_init(&attr);
	if (pthread_create(&thread, &attr, springboard_main, this)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start thread: %s",
			strerror(errno));
		throw std::runtime_error("failed to start thread");
	}

	// now that the thread is running, we should remember this in the
	// variable _running variable
	_isrunning = true;

	// leaving the start method unlocks the releases the lock, so
	// the thread can start running
}

/**
 * \brief stop the thread
 * 
 * This works by setting the _terminate variable
 */
void	ThreadBase::stop() {
	GuidingLock	lock(&mutex);

	// signal the thread that it should terminate
	_terminate = false;
}

/**
 * \brief Wait for the thread to terminate
 *
 * Termination of the thread is signaled through the cond variable, so we
 * just have to wait until it is signalled.
 */
bool	ThreadBase::wait(double timeout) {
	GuidingLock	lock(&mutex);
	double	t = Timer::gettime();
	struct timespec	ts;
	ts.tv_sec = floor(t);
	ts.tv_nsec = 1000000000 * (t - floor(t));

	// wait for the thread function to signal termination
	int	rc = pthread_cond_timedwait(&waitcond, &mutex, &ts);
	return (rc == 0);
}

/**
 * \brief run function
 */
void	ThreadBase::run() {
	// for the thread to be released
	pthread_mutex_lock(&mutex);
	_terminate = false;

	// run the main methodrun the main method
	pthread_mutex_unlock(&mutex);
	this->main();
	
	// when the main function terminates, we signal this to all
	// waiting clients
	pthread_cond_broadcast(&waitcond);
}

//////////////////////////////////////////////////////////////////////
// GuidingProcess base class methods
//////////////////////////////////////////////////////////////////////
GuidingProcess::GuidingProcess(Guider& guider, TrackerPtr tracker)
	: _guider(guider), _tracker(tracker) {
}

} // namespace guiding
} // namespace astro
