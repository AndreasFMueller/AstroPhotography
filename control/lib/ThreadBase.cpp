/*
 * ThreadBase.cpp -- base thread class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuidingProcess.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {
namespace guiding {

/**
 * \brief Accessor class to the run method of the thread
 *
 * Since we want the run run and main methods to be private, we need a
 * way to make them accessible to the global function used to launch the
 * thread. This class implements a public main function that in turn 
 * runs the run method. This class is declared a friend of the ThreadBase
 * class, so it can run the private run method.
 */
class RunAccess {
	ThreadBase&	_threadbase;
public:
	RunAccess(ThreadBase& threadbase) : _threadbase(threadbase) { }
	void	main() { _threadbase.run(); }
};

//////////////////////////////////////////////////////////////////////
// ThreadBase methods
//////////////////////////////////////////////////////////////////////

/**
 * \brief generic springboard function
 *
 * This function uses the RunAccess class to get access to the run method
 * of the thread class handed in as the argument.
 */
static void	*springboard_main(void *threadbase) {
	ThreadBase	*g = (ThreadBase *)threadbase;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main function starts");
	RunAccess(*g).main();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main function terminates");
	return threadbase;
}

/**
 * \brief construct a ThreadBase
 *
 * This mostly means setting up the mutex and condition variables 
 */
ThreadBase::ThreadBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create new ThreadBase");
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
 *
 * Stop the thread, then wait for its termination
 * derived classes may alreay have stopped the thread
 */
ThreadBase::~ThreadBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy ThreadBase");
	if (!isrunning()) {
		return;
	}
	// during the following try block we should not lock anything,
	// because the thread may want to do it's own locking
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop running thread");
		stop();
		void	*result;
		pthread_join(thread, &result);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread has terminated");
	} catch (const std::exception& x) {
		std::string	msg = stringprintf(
			"error in ThreadBase destructor: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop request to thread %p", thread);

	// signal the thread that it should terminate
	_terminate = true;
}

/**
 * \brief Wait for the thread to terminate
 *
 * Termination of the thread is signaled through the cond variable, so we
 * just have to wait until it is signalled.
 */
bool	ThreadBase::wait(double timeout) {
	GuidingLock	lock(&mutex);
	if (!isrunning()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"thread has terminated already, no wait needed");
		return true;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for thread %p to stop", thread);
	double	t = Timer::gettime() + timeout;
	struct timespec	ts;
	ts.tv_sec = floor(t);
	ts.tv_nsec = 1000000000 * (t - floor(t));
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"wait at most until %d.%09d (%f seconds)",
		ts.tv_sec, ts.tv_nsec, timeout);

	// wait for the thread function to signal termination
	int	rc = pthread_cond_timedwait(&waitcond, &mutex, &ts);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait complete: rc = %d", rc);
	return (rc == 0);
}

/**
 * \brief run function
 *
 * This method is accessible by the RunAccess class and is used in the
 * springboard function to launch the thread main
 */
void	ThreadBase::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "run the thread main function");
	// run the main methodrun the main method
	this->main();
	
	// when the main function terminates, we signal this to all
	// waiting clients
	pthread_cond_broadcast(&waitcond);

	// and we remember that we are not running
	{
		GuidingLock	lock(&mutex);
		_isrunning = false;
	}
}

} // namespace guiding
} // namespace astro
