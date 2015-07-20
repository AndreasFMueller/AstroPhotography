/*
 * Mount.cpp -- mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <stdexcept>

namespace astro {
namespace device {

DeviceName::device_type Mount::devicetype = DeviceName::Mount;

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
 * \brief Cancel a movement command
 */
void	Mount::cancel() {
}

/**
 * \brief Convert mount state type into a string
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

} // namespace device
} // namespace astro
