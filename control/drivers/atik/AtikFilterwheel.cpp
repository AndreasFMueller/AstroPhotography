/*
 * AtikFilterwheel.cpp -- implementation of the ATIK filterwheel
 *
 * (c) 2106 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikFilterwheel.h>
#include <AtikUtils.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Create a filter wheel
 *
 * \param camera	camera to which this filterwheel is attached
 */
AtikFilterwheel::AtikFilterwheel(AtikCamera& camera)
	: FilterWheel(astro::DeviceName(camera.name(),
		astro::DeviceName::Filterwheel)),
	  _camera(camera) {
	// query the filterwheel
	query();

	// start the monitoring thread
	_running = true;
	_thread = std::thread(main, this);
}

/**
 * \brief The run method that monitors the filterwheel
 */
void	AtikFilterwheel::run() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	FilterWheel::State	oldstate = getState();
	unsigned int	oldposition = currentPosition();
	while (_running) {
		// query the filterwheel state
		query();

		// check whether the state has changed
		FilterWheel::State	newstate = getState();
		if (newstate != oldstate) {
			callback(newstate);
			oldstate = newstate;
		}
		unsigned int	newposition = currentPosition();
		if (newposition != oldposition) {
			callback(newposition);
			oldposition = newposition;
		}

		// wait for a change
		_condition.wait_for(lock, std::chrono::milliseconds(100));
	}
}

/**
 * \brief static trampoline function to launch the monitor thread
 *
 * \param fw	the filterwheel to monitor
 */
void	AtikFilterwheel::main(AtikFilterwheel *fw) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	try {
		fw->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread crashed: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread crashed");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread terminates");
}

/**
 * \brief Stop the monitoring thread
 */
void	AtikFilterwheel::stop() {
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		_running = false;
	}
	_condition.notify_all();
	if (_thread.joinable()) {
		_thread.join();
	}
}

/**
 * \brief Find the number of filters
 *
 * \return the number filters this camera has
 */
unsigned int	AtikFilterwheel::nFilters0() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	return filtercount;
}

/**
 * \brief Get the current position
 *
 * \return filter position (integer between 0 - (nFilters()-1) )
 */
unsigned int	AtikFilterwheel::currentPosition() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	return current;
}

/**
 * \brief Get the current state of the filterwheel
 *
 * \return current filter wheel state
 */
FilterWheel::State	AtikFilterwheel::getState() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (moving) {
		return FilterWheel::moving;
	}
	return FilterWheel::idle;
}

/**
 * \brief Query the filterwheel for its current state
 */
void	AtikFilterwheel::query() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	_camera.getFilterWheelStatus(&filtercount, &moving, &current, &target);
}

/**
 * \brief Select a new filter
 *
 * \param filterindex	filter to select (0 - (nFilters()-1) )
 */
void	AtikFilterwheel::select(size_t filterindex) {
	_camera.setFilter(filterindex);
	_condition.notify_all();
}

/**
 * \brief Get the user friendly name of the filter wheel
 */
std::string	AtikFilterwheel::userFriendlyName() const {
	return _camera.userFriendlyName();
}

} // namespace atik
} // namespace camera
} // namespace stro
