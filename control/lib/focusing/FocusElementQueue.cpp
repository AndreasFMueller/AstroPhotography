/*
 * FocusElementQueue.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief Create a new focus element queue
 */
FocusElementQueue::FocusElementQueue() : _terminated(false) {
}

/**
 * \brief Put the focus element into the queue
 */
void	FocusElementQueue::put(FocusElementPtr feptr) {
	std::unique_lock<std::mutex>	lock(_mutex);
	if (_terminated) {
		throw std::runtime_error("queue already terminated");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "put new element into the queue %s",
		feptr->toString().c_str());
	push(feptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "queue has now %d entries", size());
	_condition.notify_all();
}

/**
 * \brief Terminate the queue
 */
void	FocusElementQueue::terminate() {
	std::unique_lock<std::mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "terminate() called");
	_terminated = true;
	_condition.notify_all();
}

/**
 * \brief Get the next queue element
 *
 * If there is no new data, the method waits until new data is put into the
 * queue. If the queue is terminated, a Null pointer is returned.
 */
FocusElementPtr	FocusElementQueue::get() {
	std::unique_lock<std::mutex>	lock(_mutex);
	while (1) {
		if (!empty()) {
			FocusElementPtr	fe = front();
			pop();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new element %s found",
				fe->toString().c_str());
			return fe;
		}
		// if we get to this point, the queue is empty, so we
		// have to wait. But before that, we check whether we
		// should terminate
		if (_terminated) {
			return FocusElementPtr();
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for new element");
		_condition.wait(lock);
	}
}

} // namespace focusing
} // namespace astro
