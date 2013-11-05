/*
 * SimCooler.cpp -- Cooler Simulator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimCooler.h>
#include <SimUtil.h>
#include <includes.h>
#include <AstroDebug.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {

#define AMBIENT_TEMPERATURE     (273 + 13.2)

SimCooler::SimCooler(SimLocator& locator)
	: Cooler(DeviceName("cooler:simulator/cooler")), _locator(locator) {
	temperature = AMBIENT_TEMPERATURE;
	lasttemperature = AMBIENT_TEMPERATURE;
	laststatechange = simtime();
	on = false;
}

float	SimCooler::getActualTemperature() {
	double	timepast = simtime() - laststatechange;
	float	targettemperature = (on) ? temperature : AMBIENT_TEMPERATURE;
	float	delta = targettemperature - lasttemperature;
	if (timepast < 5) {
		return (timepast / 6) * delta + lasttemperature;
	}
	return targettemperature - exp(5 - timepast) * delta / 6;
}

void	SimCooler::setTemperature(float _temperature) {
	lasttemperature = getActualTemperature();
	laststatechange = simtime();
	temperature = _temperature;
}

void	SimCooler::setOn(bool onoff) {
	if (onoff == on) {
		return;
	}
	lasttemperature = getActualTemperature();
	laststatechange = simtime();
	on = onoff;
}

int	SimCooler::belowambient() {
	int	result = (AMBIENT_TEMPERATURE - getActualTemperature()) / 7.;
	return result;
}

} // namespace simulator
} // namespace camera
} // namespace atro
