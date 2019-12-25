/*
 * Temperature.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>

namespace astro {

const float	Temperature::zero = 273.15;

/**
 * \brief Create a temperature classe
 *
 * \param temperature	the temperature value
 * \param scale		the temperature scale
 */
Temperature::Temperature(float temperature, temperature_scale scale) {
	switch (scale) {
	case KELVIN:	_temperature = temperature;
			break;
	case CELSIUS:	_temperature = temperature + zero;
			break;
	}
}

/**
 * \brief The celsius temperature
 */
float	Temperature::celsius() const {
	return _temperature - zero;
}

} // namespace astro
