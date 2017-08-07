/*
 * Guide.cpp -- Guide class for guiding state
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <stdexcept>

namespace astro {
namespace guiding {

Guide::state	Guide::string2state(const std::string& s) {
	if (s == "unconfigured") {
		return unconfigured;
	}
	if (s == "idle") {
		return idle;
	}
	if (s == "calibrating") {
		return calibrating;
	}
	if (s == "calibrated") {
		return calibrated;
	}
	if (s == "guiding") {
		return guiding;
	}
	if (s == "darkacquire") {
		return darkacquire;
	}
	if (s == "flatacquire") {
		return flatacquire;
	}
	if (s == "imaging") {
		return imaging;
	}
	if (s == "backlash") {
		return backlash;
	}
	throw std::runtime_error("unknown state string");
}

std::string	Guide::state2string(state s) {
	switch (s) {
	case unconfigured:
		return std::string("unconfigured");
	case idle:
		return std::string("idle");
	case calibrating:
		return std::string("calibrating");
	case calibrated:
		return std::string("calibrated");
	case guiding:
		return std::string("guiding");
	case darkacquire:
		return std::string("darkacquire");
	case flatacquire:
		return std::string("flatacquire");
	case imaging:
		return std::string("imaging");
	case backlash:
		return std::string("backlash");
	}
	throw std::runtime_error("unknown state ");
}

} // namespace guiding
} // namespace astro
