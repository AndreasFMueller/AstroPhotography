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
	: Cooler(coolername(camera)), _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating ATIK cooler");
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
}

/**
 * \brief Get the set temperature
 *
 * \return temperature in Kelvin (absolute temperature)
 */
float	AtikCooler::getSetTemperature() {
	try {
		return _lastSetTemperature = _camera.getSetTemperature(*this);
	} catch (...) {
		return _lastSetTemperature;
	}
}

/** 
 * \brief Get the actual temperature
 *
 * \return temperature in Kelvin (absolute temperature)
 */
float	AtikCooler::getActualTemperature() {
	try {
		return _lastTemperature = _camera.getActualTemperature(*this);
	} catch (...) {
		return _lastTemperature;
	}
}

/**
 * \brief Set the temperature for the cooler
 *
 * \param temperature 	temperature to set in Kelvin (absolute temperature)
 */
void	AtikCooler::setTemperature(const float temperature) {
	_camera.setTemperature(temperature, *this);
	_lastSetTemperature = temperature;
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
}

} // namespace atik
} // namespace camera
} // namespace astro
