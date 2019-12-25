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

DeviceName	coolername(const DeviceName& cameraname) {
	return cameraname
			.child(DeviceName::Ccd, "ccd")
			.child(DeviceName::Cooler, "cooler");
}

/**
 * \brief Create the QsiCooler
 *
 * \param camera	
 */
QsiCooler::QsiCooler(QsiCamera& camera)
	: Cooler(coolername(camera.name())), _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing a QsiCooler");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex);

	// get the temperature, this initializes the _actual_temperature
	getActualTemperature();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature: %.1f",		
		_actual_temperature - Temperature::zero);
	_is_on = false;
}

/**
 * \brief Get the set temperature
 *
 * \return Set absolute temperature of the cooler 
 */
float	QsiCooler::getSetTemperature() {
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
		return temp + Temperature::zero;
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
float	QsiCooler::getActualTemperature() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "getting actual temperature");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return _actual_temperature;
	}
	try {
		double	temp;
		START_STOPWATCH;
		_camera.camera().get_CCDTemperature(&temp);
		END_STOPWATCH("get_CCDTemperature()");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got temperature %.1f", temp);
		_actual_temperature = temp + Temperature::zero;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "could not get actual "
			"temperature: %s", x.what());
	}
	return _actual_temperature;
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
}

/**
 * \brief Find out whether the cooler is on
 */
bool	QsiCooler::isOn() {
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "checking cooler state");
	std::unique_lock<std::recursive_mutex>	lock(_camera.mutex,
		std::try_to_lock);
	if (!lock) {
		return _is_on;
	}
	try {
		bool	cooleron;
		START_STOPWATCH;
		_camera.camera().get_CoolerOn(&cooleron);
		END_STOPWATCH("get_CoolerOn()");
		if (_is_on != cooleron) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler now %s",
				(cooleron) ? "on" : "off");
		}
		_is_on = cooleron;
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot check cooler: %s",
			x.what());
	}
	return _is_on;
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
	_is_on = onoff;
	START_STOPWATCH;
	_camera.camera().put_CoolerOn(onoff);
	END_STOPWATCH("put_CoolerOn()");
}

} // namespace qsi
} // namespace camera
} // namespace astro
