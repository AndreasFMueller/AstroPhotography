/*
 * Mount.cpp -- mount implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>

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

} // namespace device
} // namespace astro
