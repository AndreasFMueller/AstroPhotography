/*
 * Mount.cpp -- mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroIO.h>
#include <stdexcept>

namespace astro {
namespace device {

DeviceName::device_type Mount::devicetype = DeviceName::Mount;

/**
 * \brief Construct a mount from the stringified name
 */
Mount::Mount(const std::string& name) : Device(name, DeviceName::Mount) {
	propertySetup();
}

/**
 * \brief Construct a mount from the structured name
 */
Mount::Mount(const DeviceName& name) : Device(name, DeviceName::Mount) {
	propertySetup();
}

/**
 * \brief Prepare the properties
 *
 * Every mount has longitude and latitude associated with it
 */
void	Mount::propertySetup() {
	// parameter properties
	ParameterDescription	longitude_desc("longitude", -180, 180);
	add(longitude_desc);
	ParameterDescription	latitude_desc("latitude", -90, 90);
	add(latitude_desc);

	// we assume to have a location
	_has_location = true;

	// get the position from the device.properties
	float	longitude = 0;
	if (hasProperty("longitude")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found longitude property");
		longitude = std::stod(getProperty("longitude"));
		_location.longitude() = Angle(longitude, Angle::Degrees);
	} else {
		longitude = 8.83; // Altendorf
		_has_location = false;
	}
	parameter("longitude", longitude);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set longitude parameter to %s",
		parameterValueString("longitude").c_str());

	float	latitude = 0;
	if (hasProperty("latitude")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found latitude property");
		latitude = std::stod(getProperty("latitude"));
		_location.latitude() = Angle(latitude, Angle::Degrees);
	} else {
		latitude = 47.19; // Altendorf
		_has_location = false;
	}
	parameter("latitude", latitude);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set latitude parameter to %s",
		parameterValueString("latitude").c_str());

}

/**
 * \brief Get current mount position in RA and DEC
 */
RaDec	Mount::getRaDec() {
	throw std::runtime_error("getRaDec not implemented");
}

/**
 * \brief Get current mount position in azimut and eleveation
 */
AzmAlt	Mount::getAzmAlt() {
	throw std::runtime_error("getAzmAlt not implemented");
}

/**
 * \brief Move mount to new position in RA and DEC
 */
void	Mount::Goto(const RaDec& /* radec */) {
	throw std::runtime_error("Goto not implemented");
}

/**
 * \brief Move mount to new position in azimut and elevation
 */	
void	Mount::Goto(const AzmAlt& /* azmalt */) {
	throw std::runtime_error("Goto not implemented");
}

/**
 * \brief Find out on which side of the mount the telescope currently is
 */
bool	Mount::telescopePositionEast() {
	// use the hour angle to decide whether the telescope is on the
	// east or west
	AzmAltConverter	azmaltconverter(location());
	Angle	hourangle = azmaltconverter.hourangle(this->getRaDec());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "hour angle: %.4f", hourangle.hours());
	return (hourangle > 0);
}

/**
 * \brief Use the position 
 */
LongLat	Mount::location() {
	if (_has_location) {
		return _location;
	}
	throw std::runtime_error("position not available");
}

/**
 * \brief protected method to set the location
 *
 * This can be used in a derived class to set the location e.g. from a
 * GPS receiver attached to the telescope
 */
void	Mount::location(const LongLat& l) {
	_has_location = true;
	_location = l;
}

/**
 * \brief Cancel a movement command
 */
void	Mount::cancel() {
}

/**
 * \brief Convert mount state type into a string
 *
 * \param	state code
 * \return	string representation of the state
 */
std::string	Mount::state2string(Mount::state_type s) {
	switch (s) {
	case IDLE:
		return std::string("idle");
	case ALIGNED:
		return std::string("aligned");
	case TRACKING:
		return std::string("tracking");
	case GOTO:
		return std::string("goto");
	}
	throw std::runtime_error("undefined mount state code");
}

/**
 * \brief Convert mount state string into state code
 *
 * \param s	string representaton of the state
 * \return	state code
 */
Mount::state_type	Mount::string2state(const std::string& s) {
	if (s == "idle") {
		return IDLE;
	}
	if (s == "aligned") {
		return ALIGNED;
	}
	if (s == "tracking") {
		return TRACKING;
	}
	if (s == "goto") {
		return GOTO;
	}
	throw std::runtime_error("undefined mount state name");
}

/**
 * \brief Add the current position information to the image
 *
 * \param image		image to add the meta data
 */
void	Mount::addPositionMetadata(astro::image::ImageBase& image) {
	RaDec   position = getRaDec();
	image.setMetadata(astro::io::FITSKeywords::meta("RACENTR", 
		position.ra().degrees()));
	image.setMetadata(astro::io::FITSKeywords::meta("DECCENTR", 
		position.dec().degrees()));
	try {
		AzmAlt	direction = getAzmAlt();
		image.setMetadata(astro::io::FITSKeywords::meta("TELALT",
			direction.alt().degrees()));
		image.setMetadata(astro::io::FITSKeywords::meta("TELAZ",
			direction.azm().degrees()));
	} catch (...) {
	}
	if (hasParameter("latitude")) {
		image.setMetadata(astro::io::FITSKeywords::meta("LATITUDE",
			parameterValueFloat("latitude")));
	}
	if (hasParameter("longitude")) {
		image.setMetadata(astro::io::FITSKeywords::meta("LONGITUD",
			parameterValueFloat("longitude")));
	}
}

} // namespace device
} // namespace astro
