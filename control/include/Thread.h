/*
 * Thread.h -- common classes for thread management
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Thread_h
#define _Thread_h

#include <AstroDebug.h>
#include <includes.h>
#include <memory>
#include <AstroFormat.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {
namespace thread {

// the RunAccess class is used to work around the access restrictions to
// the run method. We want to be sure that only ThreadBase class can access
// the run method, but the thread main function must have access, so we
// mediate access to it through the RunAccess class, which will be a friend
// of the ThreadBase class.
class RunAccess;

/**
 * \brief Class encapsulating the thread stuff
 */
class ThreadBase {
	// default construction of the thread does not start execution
	std::thread	thread;
private:
	// the condition variable is used for waiting on the thread to
	// complete. If one wants to wait for the thread to complete,
	// one waits on the condition variable until the run method signals
	// that main has terminated.
	std::condition_variable_any	waitcond;
	std::recursive_mutex	mutex;
protected:
	volatile bool	_isrunning;
public:
	bool	isrunning();
protected:
	/**
	 * \brief Main method for the thread
	 *
	 * The thread will ultimately run this method from the run method
	 * defined below. This allows the thread to perform some
	 * initializations, e.g. the _isrunning and _terminate variables.
	 */
	virtual	void	main() = 0;
private:
	// The RunAccess adapter class allows to get access
	// to the protected main function from a C function
	friend class RunAccess;
	void	run();

	// signaling boolean variable that can be use to signal the thread
	// to terminate
private:
	volatile bool	_terminate;
public:
	bool	terminate() const { return _terminate; }
private:	// prevent copying of this object
	ThreadBase(ThreadBase& other);
	ThreadBase&	operator=(ThreadBase& other);
public:
	ThreadBase();
	virtual ~ThreadBase();
	void	start();
	void	stop();
	bool	wait(double timeout);
};
typedef std::shared_ptr<ThreadBase>	ThreadPtr;

/**
 * \brief Generic thread template
 *
 * This template implements the "mechanical" aspects of a thread. The work
 * is encapsulated in the Work template argument. A Work class must implement
 * the main method, which is called with the thread object as argument
 * The main function can use the thread object for synchronization and for
 * signaling.
 */
template<typename Work>
class Thread : public ThreadBase {
	Work	*_work;
public:
	Thread(Work *work) : _work(work) { }
protected:
	virtual void	main() {
		_work->main(*this);
	}
public:
	virtual ~Thread() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy thread for %s",
			demangle(typeid(Work).name()).c_str());
	}
};

} // namespace thread
} // namespace astro

#endif /* _Thread_h */
