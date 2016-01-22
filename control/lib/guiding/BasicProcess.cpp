/*
 * BasicProcess.cpp -- guiding processes base clasess
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <BasicProcess.h>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroFormat.h>

namespace astro {
namespace guiding {

static GuiderBase	*nonnull(GuiderBase *guider) {
	if (NULL == guider) {
		throw std::runtime_error("missing Guider ptr");
	}
	return guider;
}

/**
 * \brief whether or not the process has a guider configured
 */
bool	BasicProcess::hasGuider() const {
	return (NULL != _guider) ? true : false;
}

/**
 * \brief access the guider
 *
 * This method throws an exception when the guider is not configured, this
 * prevents segmentation faults.
 */
GuiderBase	*BasicProcess::guider() {
	if (NULL == _guider) {
		throw std::runtime_error("guider not set");
	}
	return _guider;
}

/**
 * \brief Create a new BasicProcess based on a guider
 *
 * Creating the process does not create the thread associated with this
 * process. This has to be done in the derived class constructor, because
 * only the derived class knows the work function that must be executed
 * by the thread.
 */
BasicProcess::BasicProcess(GuiderBase *guider, TrackerPtr tracker,
	persistence::Database database)
	: _guider(nonnull(guider)),
	  _exposure(guider->exposure()), _imager(guider->imager()),
	  _tracker(tracker), _database(database) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"construct a guiding process: exposure %s",
		_guider->exposure().toString().c_str());
}

/**
 * \brief Create a basic process based on indivual components
 */
BasicProcess::BasicProcess(const astro::camera::Exposure& exposure,
	camera::Imager& imager, TrackerPtr tracker,
	persistence::Database database)
	: _guider(NULL), _exposure(exposure), _imager(imager),
	  _tracker(tracker), _database(database) {

}

/**
 * \brief Start the thread associated with this process
 */
void	BasicProcess::start() {
	if (!_thread) {
		throw std::runtime_error("no thread, cannot start");
	}
	_thread->start();
}

/**
 * \brief Stop the thread associated with this process
 */
void	BasicProcess::stop() {
	if (!_thread) {
		throw std::runtime_error("no thread, cannot stop");
	}
	_thread->stop();
}

/**
 * \brief Wait for the thread associated with this process to terminate
 */
bool	BasicProcess::wait(double timeout) {
	if (!_thread) {
		throw std::runtime_error("no thread, cannot wait");
	}
	return _thread->wait(timeout);
}

} // namespace guiding
} // namespace astro
