/*
 * Cooler.cpp -- cooler implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <stdexcept>
#include <AstroFormat.h>
#include <unistd.h>
#include <AstroDebug.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::io;
using namespace astro::device;

namespace astro {
namespace camera {

DeviceName::device_type	Cooler::devicetype = DeviceName::Cooler;

/**
 * \brief auxiliary function to produce a default 
 */
DeviceName	Cooler::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Cooler, unitname);
}

/**
 * \brief Create a cooler from the name
 */
Cooler::Cooler(const DeviceName& name) : Device(name, DeviceName::Cooler) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create cooler named %s",
		Device::name().name().c_str());
	temperature = 25 + 273.1;
}

/**
 * \brief Create a cooler from the unit name
 */
Cooler::Cooler(const std::string& name) : Device(name, DeviceName::Cooler) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create cooler named %s",
		Device::name().name().c_str());
	temperature = 25 + 273.1;
}

/**
 * \brief Destroy the cooler
 */
Cooler::~Cooler() {
}

/**
 * \brief Get the current set temperature
 */
float	Cooler::getSetTemperature() {
	return temperature;
}

/**
 * \brief Retrieve the actual temperature
 *
 * Not all coolers can report the actual temperature. This method must
 * be overridden by concrete Cooler implementations.
 */
float	Cooler::getActualTemperature() {
	throw std::runtime_error("cannot measure temperature");
}

/**
 * \brief Set the set temperature
 *
 * Temperature must be absolute, so temperatures below 0 are rejected as
 * well as temperatures above 350K, as they correspond to heaters rather
 * than coolers.
 */
void	Cooler::setTemperature(float _temperature) {
	if (_temperature < 0) {
		throw std::range_error("negative absolute temperature");
	}
	if (_temperature > 350) {
		throw std::range_error("temperature too large: heater?");
	}
	temperature = _temperature;
}

/**
 * \brief Turn the cooler on/off
 *
 * This is an empty implementation that must be overridden by driver classes.
 */
void	Cooler::setOn(bool /* onoff */) {
}

/**
 * \brief Whether or not the cooler is in
 *
 * This is a default implementation, must be overridden by driver classes.
 */
bool	Cooler::isOn() {
	return true;
}

/**
 * \brief Add temperature metadata to an image
 */
void	Cooler::addTemperatureMetadata(ImageBase& image) {
	// if the cooler is not on, then there is nothing to add to the image
	if (isOn()) {
		// add set temperature information to the image (SET-TEMP)
		image.setMetadata(
			FITSKeywords::meta(std::string("SET-TEMP"),
				this->getSetTemperature() - 273.1));
	}
	
	// add actual temperature info to the image (CCD-TEMP)
	try {
		image.setMetadata(
			FITSKeywords::meta(std::string("CCD-TEMP"),
				getActualTemperature() - 273.1));
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "actual temperature unknown: %s",
			x.what());
	}
}

/**
 * \brief Find out whether the cooler has cooled to a stable temperature
 */
bool	Cooler::stable() {
	if (!isOn()) {
		return true;
	}
	float	actualtemperature = this->getActualTemperature();
	float	delta = fabs(actualtemperature - temperature);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"T_act = %.1f, T_set = %.1f, delta = %.1f",
		actualtemperature, temperature, delta);
	return (delta < 1);
}

/**
 * \brief Wait for the cooler to cool down
 */
bool	Cooler::wait(float timeout) {
	while ((timeout > 0) && (!stable())) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for cooler");
		usleep(1000000);
		timeout -= 1;
	}
	return (timeout < 0) ? false : true;
}

} // namespace camera
} // namespace astro
