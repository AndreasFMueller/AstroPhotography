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

AtikCooler::AtikCooler(AtikCamera& camera)
	: Cooler(coolername(camera)), _camera(camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating ATIK cooler");
}

AtikCooler::~AtikCooler() {
	try {
		_camera.initiateWarmUp();
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot initiate WarmUp");
	}
}

float	AtikCooler::getSetTemperature() {
	try {
		return _lastSetTemperature = _camera.getSetTemperature(*this);
	} catch (...) {
		return _lastSetTemperature;
	}
}

float	AtikCooler::getActualTemperature() {
	try {
		return _lastTemperature = _camera.getActualTemperature(*this);
	} catch (...) {
		return _lastTemperature;
	}
}

void	AtikCooler::setTemperature(const float temperature) {
	_camera.setTemperature(temperature, *this);
	_lastSetTemperature = temperature;
}

bool	AtikCooler::isOn() {
	try {
		return _lastIsOn = _camera.isOn(*this);
	} catch (...) {
		return _lastIsOn;
	}
}

void	AtikCooler::setOn(bool onoff) {
	_camera.setOn(onoff, *this);
	_lastIsOn = onoff;
}

} // namespace atik
} // namespace camera
} // namespace astro
