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
 */
SimFilterWheel::SimFilterWheel()
	: FilterWheel(DeviceName("filterwheel:simulator/filterwheel")) {
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
 */
unsigned int	SimFilterWheel::currentPosition() {
	std::unique_lock<std::mutex>	lock(_mutex);
	// wait for the filterwheel to become idle
	switch (_currentstate) {
	case FilterWheel::idle:
		return _currentposition;
	case FilterWheel::moving:
		throw BadState("Filterwheel moving");
	case FilterWheel::unknown:
		throw BadState("Filterwheel in unknown state");
	}
	throw std::logic_error("unknown filterwheel state");
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

	{
		// lock the data structures
		std::unique_lock<std::mutex>	lock(_mutex);

		// if the filterwheel is not idle, we cannot select
		if (_currentstate != FilterWheel::idle) {
			throw BadState("bad filter state");
		}

		// change the state to moving
		_nextposition = filterindex;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moving from %d to %d", _currentposition,
		_nextposition);

	// notify all threads of the data change
	_cond.notify_all();
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
	int	movetime = 0;
	callback(_currentposition);
	callback(_currentstate);

	// initialization of the filterwheel
	if (_currentstate == FilterWheel::unknown) {
		_cond.wait_for(lock, std::chrono::seconds(3));
		_currentstate = FilterWheel::idle;
		_idle_condition.notify_all();
		callback(_currentposition);
		callback(_currentstate);
	}

	while (!_terminate) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new selection");
		if (_nextposition != _currentposition) {
			callback(_nextposition);
			_currentstate = FilterWheel::moving;
			callback(_currentstate);
			movetime = _nextposition - _currentposition;
			while (movetime <= 0) {
				movetime += 5;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "movetime = %d",
				movetime);
			_cond.wait_for(lock, std::chrono::seconds(movetime));
			_currentposition = _nextposition;
			if (_terminate) {
				return;
			}
			_currentstate = FilterWheel::idle;
			_idle_condition.notify_all();
		}
		callback(_currentstate);
		_cond.wait(lock);
	}
}

/**
 * \brief Reimplementation of wait
 *
 * This is possible because we have a more efficient way to find out whether
 * the move is complete
 *
 * \param timeout	the time to wait at most for the wait
 */
bool	SimFilterWheel::wait(float timeout) {
	int	milliseconds = 1000 * timeout;
	std::unique_lock<std::mutex>	lock(_mutex);
	return (std::cv_status::timeout == _idle_condition.wait_for(lock,
		std::chrono::milliseconds(milliseconds)));
}

} // namespace simulator
} // namespace camera
} // namespace astro
