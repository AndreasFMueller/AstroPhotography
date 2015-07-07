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
	}
	throw std::runtime_error("unknown state ");
}

} // namespace guiding
} // namespace astro