/*
 * InstrumentComponentKey.cpp -- implementation of string conversion
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDiscovery.h>
#include <AstroFormat.h>
#include <stdexcept>

namespace astro {
namespace discover {

std::string	InstrumentComponentKey::type2string(Type t) {
	switch (t) {
	case AdaptiveOptics:
		return std::string("AdaptiveOptics");
	case Camera:
		return std::string("Camera");
	case CCD:
		return std::string("CCD");
	case Cooler:
		return std::string("Cooler");
	case GuiderCCD:
		return std::string("GuiderCCD");
	case GuiderPort:
		return std::string("GuiderPort");
	case FilterWheel:
		return std::string("FilterWheel");
	case Focuser:
		return std::string("Focuser");
	case Mount:
		return std::string("Mount");
	}
	std::string	cause = stringprintf("unknown component type: %d", t);
	throw std::range_error(cause);
}

InstrumentComponentKey::Type	InstrumentComponentKey::string2type(const std::string& name) {
	if (std::string("AdaptiveOptics") == name) {
                return AdaptiveOptics;
	}
	if (std::string("Camera") == name) {
                return Camera;
	}
	if (std::string("CCD") == name) {
                return CCD;
	}
	if (std::string("Cooler") == name) {
                return Cooler;
	}
	if (std::string("GuiderCCD") == name) {
                return GuiderCCD;
	}
	if (std::string("GuiderPort") == name) {
                return GuiderPort;
	}
	if (std::string("FilterWheel") == name) {
                return FilterWheel;
	}
	if (std::string("Focuser") == name) {
                return Focuser;
	}
	if (std::string("Mount") == name) {
                return Mount;
	}
	std::string	cause = stringprintf("unknown component type %s",
		name.c_str());
	throw std::runtime_error(cause);
}

} // namespace discover
} // namespace astro
