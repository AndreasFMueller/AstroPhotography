/*
 * CancellableWork.cpp -- implementation of cancellable class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CancellableWork.h>

namespace astro {
namespace task {

/**
 * \brief Base class constructor
 *
 * This only initializes the _cancelled member
 */
CancellableWork::CancellableWork() {
	_cancelled = false;
}

/**
 * \brief Cancel the work
 */
void	CancellableWork::cancel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel work");
	_cancelled = true;
	wait_cond.notify_all();
}

/**
 * \brief Cancellation point method
 *
 * Sprinkle calls to this method throughout your run method to give the
 * task some points where it could be cancelled
 */
void	CancellableWork::cancellation_point() {
	if (cancelled()) {
		throw CancelException();
	}
}

/**
 * \brief Wait for cancellation or timeout
 */
bool	CancellableWork::wait(float t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "acquiring the ExposureTask::_lock");
	std::unique_lock<std::mutex>    lock(wait_lock);
	cancellation_point();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for %.3f seconds", t);
	long long       ns = 1000000000 * t;

	// wait for completion of exposure
	bool    timedout = (std::cv_status::no_timeout !=
		wait_cond.wait_for(lock, std::chrono::nanoseconds(ns)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait %s",
		(timedout) ? "cancelled" : "complete");
	cancellation_point();
	return timedout;
}

/**
 * \brief wait for a timeout, cancellation or a condition
 */
bool	CancellableWork::wait(float t, Condition& condition) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "acquiring the ExposureTask::_lock");
	std::unique_lock<std::mutex>    lock(wait_lock);
	cancellation_point();

	// XXX we need to keep track of the elapsed time, we want to exit the
	//     loop as soon as the timeout expires
	while (true) {
		// check whether the work has been cancelled
		cancellation_point();

		// check whether condition was met
		if (condition()) {
			return true;
		}

		// wait for one second, then try again
		long long       ns = 1000000000;
		bool    timedout = (std::cv_status::no_timeout !=
			wait_cond.wait_for(lock, std::chrono::nanoseconds(ns)));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait %s",
			(timedout) ? "cancelled" : "complete");

		// if we are not timed out, then the thread was cancelled
		if (!timedout) {
			return false;
		}

		// try again
	}
	// timeout exit
	return false;
}

} // namespace task
} // namespace astro
