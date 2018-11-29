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
 * \brief Convert filterwheel state to a readable string
 */
std::string	FilterWheel::state2string(State s) {
	switch (s) {
	case idle:
		return std::string("idle");
	case moving:
		return std::string("moving");
	case unknown:
		return std::string("unknown");
	}
	throw std::runtime_error("unknown filterwheel state code");
}

/**
 * \brief Convert filterwheel state name string to a state code
 */
FilterWheel::State	FilterWheel::string2state(const std::string& s) {
	if (s == "idle") {
		return idle;
	}
	if (s == "moving") {
		return moving;
	}
	if (s == "unknown") {
		return unknown;
	}
	throw std::runtime_error("unknown filterwheel state string");
}

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
	nfilters = std::numeric_limits<unsigned int>::max();
}

/**
 * \brief Constructor for Filterwheels
 */
FilterWheel::FilterWheel(const std::string& name)
	: Device(name, DeviceName::Filterwheel) {
	nfilters = std::numeric_limits<unsigned int>::max();
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
unsigned int	FilterWheel::nFilters0() {
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
 * \brief The public method to retrieve the number of filters
 */
unsigned int	FilterWheel::nFilters() {
	if (std::numeric_limits<unsigned int>::max() == nfilters) {
		nfilters = this->nFilters0();
	}
	return nfilters;
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
