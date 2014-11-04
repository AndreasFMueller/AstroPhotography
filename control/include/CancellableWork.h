/*
 * CancellableWork.h -- class representing work done in a thread in a
 *                      cancellable way.
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CancellableWork_h
#define _CancellableWork_h

#include <AstroTask.h>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

namespace astro {
namespace task {

/**
 * \brief Exception thrown when the thread is cancelled
 */
class CancelException : public std::exception {
public:
	CancelException() : std::exception() { }
};

/**
 * \brief Condition class used for waiting
 *
 * Ths wait method of the CancellableWork class waits until either the
 * work is cancelled, the time expires or the condition is met.
 */
class Condition {
public:
	virtual bool	operator()() = 0;
};

/**
 * \brief Cancellable work
 *
 * This class wraps around a class that does some real work in the
 * run() method. The run() method of a derived class must use the wait
 * methods in order to detect cancellation of a task.
 */
class CancellableWork {
	std::mutex	wait_lock;
	std::condition_variable	wait_cond;
	bool	_cancelled;
public:
	bool	cancelled() const { return _cancelled; }
	CancellableWork();
	virtual ~CancellableWork() { }
	virtual void	run() = 0;
	void	cancel();
protected:
	void	cancellation_point();
	bool	wait(float t);
	bool	wait(float t, Condition& condition);
};

} // namespace task
} // namespace astro

#endif /* _CancellableWork_h */
