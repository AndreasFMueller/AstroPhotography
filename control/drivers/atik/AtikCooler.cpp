/*
 * AtikCooler.cpp -- implementation of ATIK cooler class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapprswil
 */
#include <AtikCooler.h>
#include <AstroDebug.h>
#include <AtikUtils.h>

namespace astro {
namespace camera {
namespace atik {

/**
 * \brief Create a new Atik cooler
 *
 * \param camera	the camera to which this cooler belongs
 */
AtikCooler::AtikCooler(AtikCamera& camera)
	: Cooler(DeviceName(camera.name(), DeviceName::Cooler)),
	  _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating ATIK cooler");

	// start the monitoring thread
	_running = true;
	_thread = std::thread(main, this);
}

/**
 * \brief Destroy the cooler
 */
AtikCooler::~AtikCooler() {
	try {
		_camera.initiateWarmUp();
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot initiate WarmUp");
	}
	stop();
}

/**
 * \brief Get the set temperature
 *
 * \return temperature in Kelvin (absolute temperature)
 */
Temperature	AtikCooler::getSetTemperature() {
	try {
		_lastSetTemperature = _camera.getSetTemperature(*this);
	} catch (...) {
	}
	return _lastSetTemperature;
}

/** 
 * \brief Get the actual temperature
 *
 * \return temperature in Kelvin (absolute temperature)
 */
Temperature	AtikCooler::getActualTemperature() {
	try {
		_lastTemperature = _camera.getActualTemperature(*this);
	} catch (...) {
	}
	return _lastTemperature;
}

/**
 * \brief Set the temperature for the cooler
 *
 * \param temperature 	temperature to set in Kelvin (absolute temperature)
 */
void	AtikCooler::setTemperature(const float temperature) {
	_camera.setTemperature(temperature, *this);
	_lastSetTemperature = temperature;
	Cooler::setTemperature(_lastSetTemperature);
}

/**
 * \brief Find out whether the cooler is on
 *
 * \return	true if cooler is on
 */
bool	AtikCooler::isOn() {
	try {
		return _lastIsOn = _camera.isOn(*this);
	} catch (...) {
		return _lastIsOn;
	}
}

/**
 * \brief Turn the cooler on
 *
 * \param onoff	true if cooler is to be turned on
 */
void	AtikCooler::setOn(bool onoff) {
	_camera.setOn(onoff, *this);
	_lastIsOn = onoff;
	_condition.notify_all();
}

/**
 * \brief Override the temperature
 *
 * \param temperature	the new set temperature
 */
void	AtikCooler::overrideSetTemperature(float temperature) {
	Cooler::setTemperature(temperature);
}

/**
 * \brief static trampoline function to launch the thread
 *
 * \param cooler	the cooler to monitor
 */
void	AtikCooler::main(AtikCooler *cooler) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	try {
		cooler->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cooler crashed: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cooler crashed");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread terminates");
}

/**
 * \brief Do the actual monitoring
 */
void	AtikCooler::run() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	Temperature	actual = getActualTemperature();
	while (_running) {
		Temperature	newtemp = getActualTemperature();
		if (actual != newtemp) {
			// call the callback
			CoolerInfo	ci(*this);
			callback(ci);
		}
		_condition.wait_for(lock, std::chrono::seconds(3));
	}
}

/**
 * \brief Stop the monitoring thread
 */
void	AtikCooler::stop() {
	{
		std::unique_lock<std::recursive_mutex>	lock(_mutex);
		_running = false;
	}
	_condition.notify_all();
	if (_thread.joinable()) {
		_thread.join();
	}
}



} // namespace atik
} // namespace camera
} // namespace astro
