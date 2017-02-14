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

AtikCooler::AtikCooler(::AtikCamera *camera)
	: Cooler(coolername(camera)), _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating ATIK cooler");
	struct AtikCapabilities	capa;
	char	name[1024];
	CAMERA_TYPE	type;
	_camera->getCapabilities((const char **)&name, &type, &capa);
	_tempSensorCount = capa.tempSensorCount;
}

AtikCooler::~AtikCooler() {
	try {
		_camera->initiateWarmUp();
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot initiate WarmUp");
	}
}

float	AtikCooler::getSetTemperature() {
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve cooler temperature: %.1f, "
		"power %.2f, state %d", targetTemp, power, state);
	if ((state == COOLING_ON) || (state == COOLING_SETPOINT)) {
		Cooler::setTemperature(targetTemp + 273.15);
	}
	return Cooler::getSetTemperature();
}

float	AtikCooler::getActualTemperature() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve current Temp (%d)",
		_tempSensorCount);
	if (_tempSensorCount != 0) {
		float	currentTemp;
		_camera->getTemperatureSensorStatus(1, &currentTemp);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "current Temp: %.1f",
			currentTemp);
		return currentTemp + 273.15;
	};
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	switch (state) {
	case COOLING_ON:
	case COOLING_INACTIVE:
	case WARMING_UP:
		return 273.15 + 20;
	case COOLING_SETPOINT:
		return Cooler::getSetTemperature();
	}
}

void	AtikCooler::setTemperature(const float temperature) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature: %f", temperature);
	Cooler::setTemperature(temperature);
	if (isOn()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setCooling(%f)",
			temperature - 273.15);
		_camera->setCooling(temperature - 273.15);
	}
}

bool	AtikCooler::isOn() {
	COOLING_STATE	state;
	float	targetTemp = 0;
	float	power = 0;
	_camera->getCoolingStatus(&state, &targetTemp, &power);
	return ((state == COOLING_ON) || (state == COOLING_SETPOINT));
}

void	AtikCooler::setOn(bool onoff) {
	if (onoff) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn cooling on");
		_camera->setCooling(Cooler::getSetTemperature() - 273.15);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turn cooling off");
		_camera->initiateWarmUp();
	}
}

} // namespace atik
} // namespace camera
} // namespace astro
