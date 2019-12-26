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
	float	t = getActualTemperature();
	_camera.settemperature(t);
}

/**
 * \brief Destroy a cooler
 *
 * The destructor must ensure that the cooler is turned off
 */
AsiCooler::~AsiCooler() {
	try {
		setOn(false);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot turn off: %s", x.what());
	}
}

/**
 * \brief Get the set temperature
 */
float	AsiCooler::getSetTemperature() {
	return _camera.settemperature();
}

/**
 * \brief Get the current temperature
 */
float	AsiCooler::getActualTemperature() {
	return Temperature::zero + _camera.getControlValue(AsiTemperature).value / 10.;
}

/**
 * \brief Set the temperature in the camera
 */
void	AsiCooler::setCoolerTemperature() {
	AsiControlValue	value;
	value.type = AsiTargetTemp;
	// must not be multiplied by 10!
	value.value = _camera.settemperature() - Temperature::zero;
	value.isauto = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting temperature to %.1f -> %d",
		_camera.settemperature(), value.value);
	_camera.setControlValue(value);
}

/**
 * \brief Set the target temperature of the cooler
 */
void	AsiCooler::setTemperature(float temperature) {
	Cooler::setTemperature(temperature);
	_camera.settemperature(temperature);
	setCoolerTemperature();
}

/**
 * \brief Find out whether the cooler is on or off
 */
bool	AsiCooler::isOn() {
	return (_camera.getControlValue(AsiCoolerOn).value) ? true : false;
}

/**
 * \brief Turn cooler on/off
 *
 * Turning the cooler on also sets the temperature anew, because apparently
 * the camera forgets the set temperature...
 *
 * \param onoff	state of the cooler after this operation
 */
void	AsiCooler::setOn(bool onoff) {
	// turn the heater on
	AsiControlValue	value;
	value.type = AsiCoolerOn;
	value.value = (onoff) ? ASI_TRUE : ASI_FALSE;
	value.isauto = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler %s",
		onoff ? "on" : "off");
	_camera.setControlValue(value);
	// turn the fan on 
	value.type = AsiFanOn;
	_camera.setControlValue(value);
	// turn anti dew heater on
	value.type = AsiAntiDewHeater;
	_camera.setControlValue(value);
	// must send the set temperature again
	setCoolerTemperature();
}

} // namespace asi
} // namespace camera
} // namespade astro
