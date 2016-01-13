/*
 * ThreadBase.cpp -- base thread class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Thread.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <stdexcept>
#include <chrono>

namespace astro {
namespace thread {

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
static void	springboard_main(void *threadbase) {
	ThreadBase	*g = (ThreadBase *)threadbase;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main function starts");
	RunAccess(*g).main();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main function terminates");
}

/**
 * \brief construct a ThreadBase
 *
 * This mostly means setting up the mutex and condition variables 
 */
ThreadBase::ThreadBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create new ThreadBase");
	// mutex initialization
	std::unique_lock<std::recursive_mutex>	lock(mutex);

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
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"not running, no action required");
		return;
	}
	// during the following try block we should not lock anything,
	// because the thread may want to do it's own locking
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop running thread");
		stop();
		thread.join();
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
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	return _isrunning;
}

/**
 * \brief start the thread
 */
void	ThreadBase::start() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	std::unique_lock<std::recursive_mutex>	lock(mutex);

	if (isrunning()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread already running");
		throw std::runtime_error("thread already running");
	}

	_terminate = false;
	_isrunning = false;

	// start the thread
	thread = std::thread(springboard_main, this);

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping thread");
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop request to thread %p",
		thread.get_id());

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
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	if (!isrunning()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"thread has terminated already, no wait needed");
		return true;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for thread %p to stop", thread.get_id());

	long long	ns = 1000000000 * timeout;
	return std::cv_status::no_timeout
		== waitcond.wait_for(lock, std::chrono::nanoseconds(ns));
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
	try {
		this->main();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exception in thread: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown exception in thread");
	}
	
	// when the main function terminates, we signal this to all
	// waiting clients
	waitcond.notify_all();

	// and we remember that we are not running
	{
		std::unique_lock<std::recursive_mutex>	lock(mutex);
		_isrunning = false;
	}
}

} // namespace thread
} // namespace astro
