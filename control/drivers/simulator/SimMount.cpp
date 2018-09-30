/*
 * SimMount.cpp -- simulated mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimMount.h>

using namespace astro::device;

namespace astro {
namespace camera {
namespace simulator {

/**
 * \brief Construct a simulated mount
 *
 * \param locator	common simulated locator
 */
SimMount::SimMount(SimLocator& locator) 
	 : Mount(DeviceName("mount:simulator/mount")), _locator(locator) {
	ParameterDescription	longitude_desc("longitude", -180, 180);
	add(longitude_desc);
	ParameterDescription	latitude_desc("latitude", -90, 90);
	add(latitude_desc);

	// get the position from the device.properties
	float   longitude = 0;
        if (hasProperty("longitude")) {
                longitude = std::stod(getProperty("longitude"));
        } else {
                longitude = 8.83; // Altendorf
        }
        parameter("longitude", longitude);

	float   latitude = 0;
        if (hasProperty("latitude")) {
                latitude = std::stod(getProperty("latitude"));
        } else {
                latitude = 47.19; // Altendorf
        }
        parameter("latitude", latitude);
}

/**
 * \brief Determine the state of the mount
 */
Mount::state_type	SimMount::state() {
	return Mount::IDLE;
}

/**
 * \brief Get the direction into which the mount is pointing
 */
RaDec	SimMount::getRaDec() {
	return _direction;
}

/**
 * \brief Get the azimuth and altitude 
 *
 * This method always throws an exception
 */
AzmAlt	SimMount::getAzmAlt() {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("XXX cannot get AzmAlt");
}

/**
 * \brief Move to a new position in right ascension and declination
 */
void	SimMount::Goto(const RaDec& radec) {
	_direction = radec;
}

/**
 * \brief Move to a new position in azimuth and altitude
 *
 * This method always throws an exception
 */
void	SimMount::Goto(const AzmAlt& /* azmalt */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot get AzmAlt");
	throw std::runtime_error("XXX cannot goto AzmAlt");
}

/**
 * \brief Cancel movement
 *
 * This method is not implemented
 */
void	SimMount::cancel() {
	// XXX implementation missing
}

/**
 * \brief set a parameter by name
 *
 * \param name		name of the parameter
 * \param angle		angle in degrees of the parameter
 */
void	SimMount::parameter(const std::string& name, float angle) {
	if (name == std::string("longitude")) {
		_position.longitude().degrees(angle);
		return;
	}
	if (name == std::string("latitude")) {
		_position.latitude().degrees(angle);
		return;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "no parameter %s, %f",
		name.c_str(), angle);
	throw std::logic_error("no such parameter");
}

/**
 * \brief get the value of a parameter
 *
 * \param name		name of the parameter
 */
float	SimMount::parameterValueFloat(const std::string& name) const {
	if (name == std::string("longitude")) {
		return _position.longitude().degrees();
	}
	if (name == std::string("latitude")) {
		return _position.latitude().degrees();
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "no parameter named '%s'", name.c_str());
	throw std::logic_error("no such parameter");
}


} // namespace simulator
} // namespace camera
} // namespace astro
