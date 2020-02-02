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
	state(Mount::TRACKING);
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

	float	latitude = 0;
	if (hasProperty("latitude")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found latitude property");
		latitude = std::stod(getProperty("latitude"));
		_location.latitude() = Angle(latitude, Angle::Degrees);
	} else {
		latitude = 47.19; // Altendorf
		_has_location = false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "location: %s",
		_location.toString().c_str());
}

/**
 * \brief Get current mount position in RA and DEC
 */
RaDec	Mount::getRaDec() {
	debug(LOG_ERR, DEBUG_LOG, 0, "base mount has no getRaDec");
	throw std::runtime_error("getRaDec not implemented");
}

/**
 * \brief Get current mount position in azimut and eleveation
 */
AzmAlt	Mount::getAzmAlt() {
	debug(LOG_ERR, DEBUG_LOG, 0, "base mount has not getAzmAlt");
	throw std::runtime_error("getAzmAlt not implemented");
}

/**
 * \brief Move mount to new position in RA and DEC
 */
void	Mount::Goto(const RaDec& /* radec */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "base Mount cannot Goto");
	throw std::runtime_error("Goto not implemented");
}

/**
 * \brief Move mount to new position in azimut and elevation
 */	
void	Mount::Goto(const AzmAlt& /* azmalt */) {
	debug(LOG_ERR, DEBUG_LOG, 0, "base Mount cannot Goto");
	throw std::runtime_error("Goto not implemented");
}

/**
 * \brief Find out on which side of the mount the telescope currently is
 */
bool	Mount::telescopePositionWest() {
	try {
		// use the hour angle to decide whether the telescope is on the
		// east or west
		AzmAltConverter	azmaltconverter(location());
		Angle	hourangle = azmaltconverter.hourangle(this->getRaDec());
		return (hourangle <= 0);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get telescope "
			"position: %s", x.what());
	}
	return true;
}

/**
 * \brief Use the position 
 */
LongLat	Mount::location() {
	if (_has_location) {
		return _location;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "called Mount::location without location");
	throw std::runtime_error("position not available");
}

/**
 * \brief Get the location source
 */
Mount::location_source_type	Mount::location_source() {
	return Mount::LOCAL;
}

/**
 * \brief Get the time from the mount
 *
 * In most cases, this is just the system time. But if e.g. a Celestron mount
 * has a GPS device attached, then this value of the time will be more
 * reliable
 */
time_t	Mount::time() {
	time_t	now;
	::time(&now);
	return now;
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
 * \brief The default mount does not have guide rates
 */
bool	Mount::hasGuideRates() {
	return false;
}

/**
 * \brief Get the (useless) guide rates
 */
RaDec	Mount::getGuideRates() {
	return RaDec(Angle(0), Angle(0));
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding mount metadata to image");
	RaDec   position = getRaDec();
	image.setMetadata(astro::io::FITSKeywords::meta("RACENTR", 
		position.ra().hours()));
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
	image.setMetadata(astro::io::FITSKeywords::meta("LATITUDE",
		_location.latitude().degrees()));
	image.setMetadata(astro::io::FITSKeywords::meta("LONGITUD",
		_location.longitude().degrees()));
}

/**
 * \brief Add a callback for state changes
 */
void	Mount::addStatechangeCallback(callback::CallbackPtr callback) {
	_statechangecallback.insert(callback);
}

/**
 * \brief Remove a callback
 */
void	Mount::removeStatechangeCallback(callback::CallbackPtr callback) {
	auto	i = _statechangecallback.find(callback);
	if (i != _statechangecallback.end()) {
		_statechangecallback.erase(i);
	}
}

/**
 * \brief Add a callback for state changes
 */
void	Mount::addPositionCallback(callback::CallbackPtr callback) {
	_positioncallback.insert(callback);
}

/**
 * \brief Remove a callback
 */
void	Mount::removePositionCallback(callback::CallbackPtr callback) {
	auto	i = _positioncallback.find(callback);
	if (i != _positioncallback.end()) {
		_positioncallback.erase(i);
	}
}

/**
 * \brief Send state change information to the callback
 */
void	Mount::callback(state_type newstate) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state change callback: %s",
		state2string(newstate).c_str());
	callback::CallbackDataPtr	data(new StateCallbackData(newstate));
	_statechangecallback(data);
}

/**
 * \brief Send position information to the callback
 */
void	Mount::callback(const RaDec& newposition) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "position callback: %s",
		newposition.toString().c_str());
	callback::CallbackDataPtr	data(new PositionCallbackData(newposition));
	_statechangecallback(data);
}

/**
 * \brief Set state and also send state change callbac
 */
void	Mount::state(state_type s) {
	_state = s;
	callback(_state);
}

} // namespace device
} // namespace astro
