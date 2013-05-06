/*
 * Cooler.cpp -- cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <stdexcept>

namespace astro {
namespace camera {

Cooler::Cooler() {
	temperature = 25 + 273.1;
}

Cooler::~Cooler() {
}

float	Cooler::getSetTemperature() {
	return temperature;
}

float	Cooler::getActualTemperature() {
	throw std::runtime_error("cannot measure temperature");
}

void	Cooler::setTemperature(float _temperature) {
	if (_temperature < 0) {
		throw std::range_error("negative absolute temperature");
	}
	if (_temperature > 350) {
		throw std::range_error("temperature too large: heater?");
	}
	temperature = _temperature;
}

void	Cooler::setOn(bool onoff) {
}

bool	Cooler::isOn() {
	return true;
}

} // namespace camera
} // namespace astro
