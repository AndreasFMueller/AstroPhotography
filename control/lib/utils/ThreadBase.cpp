/*
 * ThreadBase.cpp -- base thread class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
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
static void	springboard_main(ThreadBase *threadbase) {
	// make sure we have a threadbase argument
	if (NULL == threadbase) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "threadbase argument is NULL");
		return;
	}
	// get the type of the threadbase for better logging
	std::string	classname = demangle(typeid(*threadbase).name());
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calling main of %s",
			classname.c_str());
		RunAccess(*threadbase).main();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "main of %s completed",
			classname.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "main of %s terminated by %s: %s",
			classname.c_str(), demangle(typeid(x).name()).c_str(),
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "main of %s terminated (unknown)",
			classname.c_str());
	}
}

/**
 * \brief construct a ThreadBase
 *
 * This mostly means setting up the mutex and condition variables 
 */
ThreadBase::ThreadBase() {
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
	// stop the thread if it is running
	if (isrunning()) {
		stop();
	}

	// wait for the thread to exit
	try {
		thread.join();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "thread %p has been joined",
			thread.get_id());
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

	// make sure the thread is not already running before starting a
	// new thread
	if (isrunning()) {
		std::string	cause = stringprintf("thread %p is running",
			thread.get_id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	if (thread.joinable()) {
		std::string	cause = stringprintf("thread %p is joinable, "
			"cannot start new thread", thread.get_id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}

	// initialize fields
	_terminate = false;
	_isrunning = true;	// not quite yet, but soon

	// start a new thread
	thread = std::thread(springboard_main, this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread %p constructed",
		thread.get_id());

	// leaving the start method unlocks the releases the lock, so
	// the thread can start running
}

/**
 * \brief stop the thread
 * 
 * This works by setting the _terminate variable
 */
void	ThreadBase::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping thread %p", thread.get_id());
	std::unique_lock<std::recursive_mutex>	lock(mutex);

	// signal the thread that it should terminate
	_terminate = true;
}

/**
 * \brief Wait for the thread to terminate
 *
 * Termination of the thread is signaled through the cond variable, so we
 * just have to wait until it is signalled.
 *
 * \return 	true if the thread has been stopped
 */
bool	ThreadBase::wait(double timeout) {
	std::unique_lock<std::recursive_mutex>	lock(mutex);
	// make sure there really is something to wait for
	if (!thread.joinable()) {
		std::string	cause = stringprintf("thread %p not joinable, "
			"no need to wait", thread.get_id());
		return true;
	}
	if (!isrunning()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"thread %p has terminated already, no wait needed",
			thread.get_id());
		return true;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for thread %p to stop",
		thread.get_id());

	long long	ns = 1000000000 * timeout;
	if (std::cv_status::no_timeout != waitcond.wait_for(lock,
		std::chrono::nanoseconds(ns))) {
		debug(LOG_ERR, DEBUG_LOG, 0, "timeout happened");
		return false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "joining thread %p", thread.get_id());
	thread.join();
	return true;
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
		debug(LOG_ERR, DEBUG_LOG, 0, "%s exception in thread %p: %s",
			demangle(typeid(x).name()).c_str(),
			std::this_thread::get_id(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown exception in thread %p",
			std::this_thread::get_id());
	}
	
	// when the main function terminates, we signal this to all
	// waiting clients
	debug(LOG_DEBUG, DEBUG_LOG, 0, "signal that thread %p has terminated",
		std::this_thread::get_id());
	waitcond.notify_all();

	// and we remember that we are not running
	{
		std::unique_lock<std::recursive_mutex>	lock(mutex);
		_isrunning = false;
	}
}

} // namespace thread
} // namespace astro
