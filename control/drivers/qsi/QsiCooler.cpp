/*
 * QsiCooler.cpp -- qsi cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiCooler.h>
#include <QsiUtils.h>

namespace astro {
namespace camera {
namespace qsi {

/**
 * \brief Create the QsiCooler
 *
 * \param camera	the camera owning this cooler
 */
QsiCooler::QsiCooler(QsiCamera& camera)
	: Cooler(DeviceName(camera.name(), DeviceName::Cooler)),
	  _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing a QsiCooler");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

	// get the temperature
	getActualTemperature();
	getSetTemperature();
	isOn();

	// start the thread
	_running = true;
	_thread = std::thread(start_main, this);
}

/**
 * \brief Destroy the cooler
 *
 * The destructor has to take care of the thread
 */
QsiCooler::~QsiCooler() {
	stop();
}

/**
 * \brief Get the set temperature
 *
 * \return Set absolute temperature of the cooler 
 */
Temperature	QsiCooler::getSetTemperature() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "getting set temperature");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return Cooler::getSetTemperature();
	}
	try {
		double	temp;
		START_STOPWATCH;
		_camera.camera().get_SetCCDTemperature(&temp);
		END_STOPWATCH("get_SetCCDTemperature()");
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "got temerature %.1f", temp);
		return Temperature(temp, Temperature::CELSIUS);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "could not get set "
			"temperature: %s", x.what());
	}
	return Cooler::getSetTemperature();
}

/**
 * \brief Get the actual temperature
 *
 * This method returns the last retrieved actual temperature if the
 * camera happens to be locked
 */
Temperature	QsiCooler::getActualTemperature() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "getting actual temperature");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return Cooler::getActualTemperature();
	}
	try {
		double	temp;
		START_STOPWATCH;
		_camera.camera().get_CCDTemperature(&temp);
		END_STOPWATCH("get_CCDTemperature()");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got temperature %.1f", temp);
		actualTemperature(temp + Temperature::zero);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "could not get actual "
			"temperature: %s", x.what());
	}
	return Cooler::getActualTemperature();
}

/**
 * \brief Set the temperature
 *
 * \param temperature	absolute temperature to set
 */
void	QsiCooler::setTemperature(const float temperature) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting temperature %.1f",
		temperature - Temperature::zero);
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	Cooler::setTemperature(temperature);
	double	temp = temperature - Temperature::zero;
	START_STOPWATCH;
	_camera.camera().put_SetCCDTemperature(temp);
	END_STOPWATCH("put_SetCCDTemperature()");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature now %.1f", temp);
	_condition.notify_all();
}

/**
 * \brief Find out whether the cooler is on
 */
bool	QsiCooler::isOn() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking cooler state");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return _on;
	}
	try {
		bool	cooleron;
		START_STOPWATCH;
		_camera.camera().get_CoolerOn(&cooleron);
		END_STOPWATCH("get_CoolerOn()");
		if (_on != cooleron) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler now %s",
				(cooleron) ? "on" : "off");
		}
		_on = cooleron;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot check cooler: %s",
			x.what());
	}
	return _on;
}

/**
 * \brief Turn the cooler on/off
 *
 * \param onoff		true if the cooler is supposed to be on
 */
void	QsiCooler::setOn(bool onoff) {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "set cooler state to %s",
	//	(onoff) ? "on" : "off");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);
	_on = onoff;
	START_STOPWATCH;
	_camera.camera().put_CoolerOn(onoff);
	END_STOPWATCH("put_CoolerOn()");
	Cooler::setOn(onoff);
}

/**
 * \brief static trampoline function to launch the cooler thread
 *
 * \param cooler	the cooler to monitor
 */
void	QsiCooler::start_main(QsiCooler *cooler) noexcept {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread");
	try {
		cooler->run();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread failed %s: %s",
			demangle_cstr(x), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "thread crashed");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread terminates");
}

/**
 * \brief Cooler monitoring method
 *
 * Only this thread every sends callbacks
 */
void	QsiCooler::run() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	Temperature	previoustemperature;
	while (_running) {
		Temperature	newtemperature = this->getActualTemperature();
		if (previoustemperature != newtemperature) {
			callback(CoolerInfo(*this));
		}
		previoustemperature = newtemperature;
		_condition.wait_for(lock, std::chrono::seconds(3));
	}
}

void	QsiCooler::stop() {
	_running = false;
	_condition.notify_all();
	if (_thread.joinable()) {
		_thread.join();
	}
}

} // namespace qsi
} // namespace camera
} // namespace astro
