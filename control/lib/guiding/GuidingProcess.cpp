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

/**
 * \brief Create a new GuidingProcess
 *
 * Creating the process does not create the thread associated with this
 * process. This has to be done in the derived class constructor, because
 * only the derived class knows the work function that must be executed
 * by the thread.
 */
GuidingProcess::GuidingProcess(Guider& guider, TrackerPtr tracker,
	persistence::Database database)
	: _guider(guider), _tracker(tracker), _database(database) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"construct a guiding process: exposure %s",
		_guider.exposure().toString().c_str());
}

/**
 * \brief Start the thread associated with this process
 */
void	GuidingProcess::start() {
	if (!_thread) {
		throw std::runtime_error("no thread, cannot start");
	}
	_thread->start();
}

/**
 * \brief Stop the thread associated with this process
 */
void	GuidingProcess::stop() {
	if (!_thread) {
		throw std::runtime_error("no thread, cannot stop");
	}
	_thread->stop();
}

/**
 * \brief Wait for the thread associated with this process to terminate
 */
bool	GuidingProcess::wait(double timeout) {
	if (!_thread) {
		throw std::runtime_error("no thread, cannot wait");
	}
	return _thread->wait(timeout);
}

} // namespace guiding
} // namespace astro
