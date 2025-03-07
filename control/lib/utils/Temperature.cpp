/*
 * Temperature.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <fstream>

namespace astro {

const float	Temperature::zero = 273.15;

/**
 * \brief Create a temperature classe
 *
 * \param temperature	the temperature value
 * \param scale		the temperature scale
 */
Temperature::Temperature(float temperature, temperature_scale scale) noexcept {
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

/**
 * \brief Get the core temperature
 */
Temperature	Temperature::core() {
#if __linux__
	std::ifstream	temp("/sys/class/thermal/thermal_zone0/temp");
	if (temp.bad() || temp.fail()) {
		throw std::runtime_error("not temperatur sensor");
	}
	float	temperature;
	temp >> temperature;
	temperature = temperature / 1000.;
	return Temperature(temperature, CELSIUS);
#else
	throw std::runtime_error("temperature not implemented");
#endif
}

/**
 * \brief Get the temperature difference
 *
 * The difference is not in itself a temperature
 */
float	Temperature::operator-(const Temperature& other) const {
	return _temperature - other.temperature();
}

/**
 * \brief Comparison operator for temperature
 */
bool	Temperature::operator>(const Temperature& other) const {
	return _temperature > other.temperature();
}

/**
 * \brief Comparison operator for temperature
 */
bool	Temperature::operator>=(const Temperature& other) const {
	return _temperature >= other.temperature();
}

/**
 * \brief Check whether the absolute temperature is positive
 */
bool	Temperature::operator>(float t) const {
	return _temperature > t;
}

/**
 * \brief allow assignment of absolute temperatures
 */
Temperature&	Temperature::operator=(float t) {
	_temperature = t;
	return *this;
}

bool	Temperature::operator==(const Temperature& other) const {
	return _temperature == other._temperature;
}

bool	Temperature::operator!=(const Temperature& other) const {
	return _temperature != other._temperature;
}

} // namespace astro
