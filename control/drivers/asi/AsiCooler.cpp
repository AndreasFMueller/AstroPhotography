/*
 * AsiCooler.cpp -- implementation of a cooler class for ASI cameras
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiCooler.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief auxiliary function to compute name of a cooler
 */
static DeviceName	asiCoolerName(AsiCcd& ccd) {
	return ccd.name().child(DeviceName::Cooler, "cooler");
}

/**
 * \brief Construct a new cooler
 */
AsiCooler::AsiCooler(AsiCamera& camera, AsiCcd& ccd)
	: Cooler(asiCoolerName(ccd)), _camera(camera) {
}

/**
 * \brief Destroy a cooler
 *
 * The destructor must ensure that the cooler is turned off
 */
AsiCooler::~AsiCooler() {
	try {
		setOn(false);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot turn off: %s", x.what());
	}
}

/**
 * \brief Get the set temperature
 */
float	AsiCooler::getSetTemperature() {
	return 273.1 + _camera.getControlValue(AsiTargetTemp).value / 10.;
}

/**
 * \brief Get the current temperature
 */
float	AsiCooler::getActualTemperature() {
	return 273.1 + _camera.getControlValue(AsiTemperature).value / 10.;
}

/**
 * \brief Set the target temperature of the cooler
 */
void	AsiCooler::setTemperature(float temperature) {
	AsiControlValue	value;
	value.type = AsiTargetTemp;
	value.value = 10 * (temperature - 273.1);
	value.isauto = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting temperature to %.1f -> %d",
		temperature, value.value);
	_camera.setControlValue(value);
}

/**
 * \brief Find out whether the cooler is on or off
 */
bool	AsiCooler::isOn() {
	return (_camera.getControlValue(AsiCoolerOn).value) ? true : false;
}

/**
 * \brief Turn cooler on/off
 */
void	AsiCooler::setOn(bool onoff) {
	AsiControlValue	value;
	value.type = AsiCoolerOn;
	value.value = (onoff) ? ASI_TRUE : ASI_FALSE;
	value.isauto = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler %s",
		onoff ? "on" : "off");
	_camera.setControlValue(value);
}

} // namespace asi
} // namespace camera
} // namespade astro
