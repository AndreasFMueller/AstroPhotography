/*
 * SimFilterWheel.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimFilterWheel.h>
#include <AstroExceptions.h>
#include <includes.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Trampoline function to start the run method of the filterwheel
 *
 * \param filterwheel	the filterwheel implementation
 */
void	SimFilterWheel::main(SimFilterWheel *filterwheel) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the filterwheel thread for %s",
		filterwheel->name().toString().c_str());
	try {
		filterwheel->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error in filterwheel thread: %s",
			x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end the filterwheel thread for %s",
		filterwheel->name().toString().c_str());
}

/**
 * \brief Construct a new Filterwheel object
 *
 * \param locator	the locator for simulator devices
 */
SimFilterWheel::SimFilterWheel(SimLocator& locator)
	: FilterWheel(DeviceName("filterwheel:simulator/filterwheel")),
	  _locator(locator) {
	_currentposition = 0;
	_nextposition = 0;
	_currentstate = FilterWheel::unknown;
	// setting the changetime to a future point of time makes sure
	// that the 
	_terminate = false;
	_thread = std::thread(main, this);
}

/**
 * \brief Destroy the filterwheel instance
 *
 * The destructor has to wait for the thread to terminate
 */
SimFilterWheel::~SimFilterWheel() {
	{
		std::unique_lock<std::mutex>	lock(_mutex);
		_terminate = true;
	}
	_cond.notify_all();
	if (_thread.joinable()) {
		_thread.join();
	}
}

/**
 * \brief Get the current filterwheel position
 *
 * This method has as a side effect to wait for the filterwheel to be idle
 */
unsigned int	SimFilterWheel::currentPosition() {
	std::unique_lock<std::mutex>	lock(_mutex);
	// wait for the filterwheel to become idle
	while (_currentstate != FilterWheel::idle) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad state, so we wait");
		_idle_condition.wait(lock);
	}
	return _currentposition;
}

/**
 * \brief Change the filterwheel selection
 *
 * This triggers movement of the filter wheel, which is simulated by setting
 * the _changetime.
 *
 * \param filterindex	the index of the filter to select
 */
void    SimFilterWheel::select(size_t filterindex) {
	// make sure the index is legal
	if (filterindex >= 5) {
		throw BadParameter("filterindex may not exceed number "
			"of filters");
	}

	// lock the data structures
	std::unique_lock<std::mutex>	lock(_mutex);

	// if the filterwheel is not idle, we cannot select
	if (_currentstate != FilterWheel::idle) {
		throw BadState("bad filter state");
	}

	// change the state to moving
	_nextposition = filterindex;
	_currentstate = FilterWheel::moving;

	// notify all threads of the data change
	_cond.notify_all();

	// update the callback
	callback(_currentstate);
}

/**
 * \brief Ask for the name of the current filter
 */
std::string     SimFilterWheel::filterName(size_t filterindex) {
	switch (filterindex) {
	case 0:	return std::string("L");
	case 1:	return std::string("R");
	case 2:	return std::string("G");
	case 3:	return std::string("B");
	case 4: return std::string("H-alpha");
	}
	throw BadParameter("illegal filter selection");
}

/**
 * \brief Get the current filterwheel state
 */
FilterWheel::State	SimFilterWheel::getState() {
	std::unique_lock<std::mutex>	lock(_mutex);
	return _currentstate;
}

/**
 * \brief The run method of the filterwheel thread
 */
void	SimFilterWheel::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep for 3 seconds until FW is ready");
	std::unique_lock<std::mutex>	lock(_mutex);
	while (!_terminate) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new loop");
		bool	check_new = true;
		// check the current state
		switch (_currentstate) {
		case FilterWheel::idle:
			// wait until something happens
			check_new = false;
			_idle_condition.notify_all();
			_cond.wait(lock);
			break;
		case FilterWheel::moving:
			// wait for move to complete
			_cond.wait_for(lock, std::chrono::seconds(5));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "move complete");
			break;
		case FilterWheel::unknown:
			// wait for fw to initialize
			_cond.wait_for(lock, std::chrono::seconds(3));
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"FilterWheel initialized");
			break;
		}
		// handle the case that the state has changed to terminate
		if (_terminate) {
			return;
		}
		// if we were moving or unknown, set the new idle state
		if (check_new) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "setting new state");
			_currentstate = FilterWheel::idle;
			_currentposition = _nextposition;
			callback(_currentstate);
			callback(_currentposition);
		}
	}
}

} // namespace simulator
} // namespace camera
} // namespace astro
