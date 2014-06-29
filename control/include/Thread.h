/*
 * Thread.h -- common classes for thread management
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Thread_h
#define _Thread_h

#include <pthread.h>
#include <AstroDebug.h>
#include <includes.h>
#include <memory>
#include <AstroFormat.h>

namespace astro {
namespace thread {

/**
 * \brief Locking class to make locking more automatic
 */
class Lock {
	pthread_mutex_t	*_mutex;
public:
	Lock(pthread_mutex_t *mutex) : _mutex(mutex) {
		if (pthread_mutex_lock(_mutex)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "locking fails: %s",
				strerror(errno));
		}
	}
	~Lock() {
		if (pthread_mutex_unlock(_mutex)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "");
		}
	}
};

class RunAccess;

/**
 * \brief Class encapsulating the thread stuff
 */
class ThreadBase {
	pthread_t	thread;
private:
	pthread_cond_t	waitcond; // waiting
	pthread_mutex_t	mutex;
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
	~ThreadBase();
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
	Work&	_work;
public:
	Thread(Work& work) : _work(work) { }
protected:
	virtual void	main() {
		_work.main(*this);
	}
};

} // namespace thread
} // namespace astro

#endif /* _Thread_h */
