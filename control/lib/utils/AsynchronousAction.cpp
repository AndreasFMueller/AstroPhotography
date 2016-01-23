/*
 * AsynchronousAction.cpp -- implementation of the async action class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {

AsynchronousAction::AsynchronousAction() {
	_busy = false;
}

AsynchronousAction::~AsynchronousAction() {
	if (worker.joinable()) {
		worker.join();
	}
}

static void	action_executor(AsynchronousAction *aa) {
	aa->execute();
}

void	AsynchronousAction::busy(bool b) {
	MutexLocker<std::mutex>	lock(mtx);
	_busy = b;
}

bool	AsynchronousAction::execute(ActionPtr action) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new action request");
	// if thread is running
	MutexLocker<std::mutex>	lock(mtx);
	if (!_busy) {
		_action = action;
		if (worker.joinable()) {
			worker.join();
		}
		_busy = true;
		worker = std::thread(action_executor, this);
		return true;
	}
	return false;
}

void	AsynchronousAction::execute() {
	if (_action) {
		try {
			_action->execute();
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0, "error in action");
		}
	} else {
		debug(LOG_ERR, DEBUG_LOG, 0, "no action present");
	}
	busy(false);
}

} // namespace astro
