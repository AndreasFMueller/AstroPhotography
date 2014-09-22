/*
 * FilterWheel.cpp -- basic filter wheel implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <unistd.h>
#include <AstroFormat.h>

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

/**
 * \brief Select a filter by name
 */
void	FilterWheel::select(const std::string& filtername) {
	unsigned int	n = nFilters();
	for (unsigned int index = 0; index < n; index++) {
		if (filterName(index) == filtername) {
			select(index);
			return;
		}
	}
	try {
		select(std::stod(filtername));
	} catch (...) {
	}
	std::string	msg = stringprintf("filter named '%s' not found",
		filtername.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get the number of filters from the properties
 */
unsigned int	FilterWheel::nFilters() {
	Properties	properties(name().toString());
	try {
		std::string	nfilterstring 
			= properties.getProperty(std::string("nfilters"));
		return std::stod(nfilterstring);
	} catch (...) {
		return 1;
	}
}

/**
 * \brief Get the filter name
 */
std::string	FilterWheel::filterName(size_t index) {
	if (index >= nFilters()) {
		std::string	msg = stringprintf("%u is too large", index);
		throw std::runtime_error(msg);
	}
	// get the properties
	Properties	properties(name().toString());
	try {
		return properties.getProperty(stringprintf("filter%u", index));
	} catch (...) {
		return stringprintf("%u", index);
	}
}

} // namespace camera
} // namespace astro
