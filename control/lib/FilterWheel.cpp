/*
 * FilterWheel.cpp -- basic filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <unistd.h>

using namespace astro::device;

namespace astro {
namespace camera {

DeviceName::device_type	FilterWheel::devicetype = DeviceName::Filterwheel;

/**
 * \brief Create the name of the filterwheel
 */
DeviceName	FilterWheel::defaultname(const DeviceName& parent,
			const std::string& unitname) {
	return DeviceName(parent, DeviceName::Filterwheel, unitname);
}

/**
 * \brief Constructor for Filterwheels
 */
FilterWheel::FilterWheel(const DeviceName& name)
	: Device(name, DeviceName::Filterwheel) {
}

/**
 * \brief Constructor for Filterwheels
 */
FilterWheel::FilterWheel(const std::string& name)
	: Device(name, DeviceName::Filterwheel) {
}

/**
 * \brief destroy the filterwheel
 */
FilterWheel::~FilterWheel() {
}

/**
 * \brief wait for the filter wheel to come ready
 *
 * \param timeout maximum time to wait for the filter wheel to become idle
 */
bool	FilterWheel::wait(float timeout) {
	while ((timeout > 0) && (getState() != idle)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for filterwheel");
		usleep(100000);
		timeout -= 0.1;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait complete");
	return (timeout < 0) ? false : true;
}

} // namespace camera
} // namespace astro
