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

const static Temperature	ambient_temperature(Temperature::zero + 13.2);

/**
 *  \brief Construct a new cooler object
 */
SimCooler::SimCooler(SimLocator& locator)
	: Cooler(DeviceName("cooler:simulator/cooler")), _locator(locator) {
	temperature = ambient_temperature;
	lasttemperature = ambient_temperature;
	laststatechange = simtime();
	on = false;
	_dewheatervalue = 0.;
}

/**
 * \brief Get the actual temperature
 */
Temperature	SimCooler::getActualTemperature() {
	double	timepast = simtime() - laststatechange;
	Temperature	targettemperature
				= (on) ? temperature : ambient_temperature;
	float	delta = targettemperature - lasttemperature;
	// for past time < 5, we let the temperature change linearly
	// at the end of 
	float	actemp = 0;
	if (timepast < 5) {
		actemp = (timepast / 6) * delta + lasttemperature.temperature();
	} else {
		actemp = targettemperature.temperature() - exp(5 - timepast) * delta / 6;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "t = %.1f, T = %.1f", timepast, actemp);
	return Temperature(actemp);
}

/**
 * \brief Set the set temperature
 *
 * \param _temperature	absolute temperature for the cooler
 */
void	SimCooler::setTemperature(float _temperature) {
	lasttemperature = getActualTemperature();
	laststatechange = simtime();
	temperature = _temperature;
}

/**
 * \brief Turn on or off the cooler
 */
void	SimCooler::setOn(bool onoff) {
	if (onoff == on) {
		return;
	}
	lasttemperature = getActualTemperature();
	laststatechange = simtime();
	on = onoff;
}

/**
 * \brief Find out whether the cooler is currently below ambient temperature
 */
int	SimCooler::belowambient() {
	int	result = (ambient_temperature - getActualTemperature()) / 7.;
	return result;
}

bool	SimCooler::hasDewHeater() {
	return true;
}

float	SimCooler::dewHeater() {
	return _dewheatervalue;
}

void	SimCooler::dewHeater(float d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new dew heater value: %.2f", d);
	_dewheatervalue = d;
}
std::pair<float, float>	SimCooler::dewHeaterRange() {
	return std::make_pair((float)0., (float)1.);
}

} // namespace simulator
} // namespace camera
} // namespace atro
