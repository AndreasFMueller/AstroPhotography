/*
 * AtikCooler.cpp -- implementation of ATIK cooler class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapprswil
 */
#include <AtikCooler.h>

namespace astro {
namespace camera {
namespace atik {

AtikCooler::AtikCooler(::AtikCamera *camera) : _camera(camera) {
}

float	AtikCooler::getSetTemperature() {
	return 0;
}

float	AtikCooler::getActualTemperature() {
	return 0;
}

void	AtikCooler::setTemperature(const float temperature) {
}

bool	AtikCooler::isOn() {
}

void	AtikCooler::setOn(bool onoff) {
}

} // namespace atik
} // namespace camera
} // namespace astro
