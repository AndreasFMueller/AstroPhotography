/*
 * Shutter.cpp -- shutter related methods
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <stdexcept>

namespace astro {
namespace camera {

std::string	Shutter::state2string(state s) {
	switch (s) {
	case CLOSED:
		return std::string("closed");
	case OPEN:
		return std::string("open");
	}
	throw std::runtime_error("unknown shutter state code");
}

Shutter::state	Shutter::string2state(const std::string& s) {
	if (s == "closed") {
		return CLOSED;
	}
	if (s == "open") {
		return OPEN;
	}
	throw std::runtime_error("unknown shutter state");
}

} // namespace camera
} // namespace astro
