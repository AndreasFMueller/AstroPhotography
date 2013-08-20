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
	: Cooler(std::string("sim-cooler")), _locator(locator) {
	temperature = AMBIENT_TEMPERATURE;
	lasttemperature = AMBIENT_TEMPERATURE;
	laststatechange = simtime();
	on = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state initialization at: %f",
		laststatechange);
}

float	SimCooler::getActualTemperature() {
	double	timepast = simtime() - laststatechange;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time past: %f", timepast);
	float	targettemperature = (on) ? temperature : AMBIENT_TEMPERATURE;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "target temperature: %f",
		targettemperature);
	float	delta = targettemperature - lasttemperature;
	if (timepast < 5) {
		return (timepast / 6) * delta + lasttemperature;
	}
	return -exp(5 - timepast) * delta + targettemperature;
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

} // namespace simulator
} // namespace camera
} // namespace atro
