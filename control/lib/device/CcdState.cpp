/*
 * CcdState.cpp -- Implementation of CCD state
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

std::string	CcdState::state2string(State s) {
	switch (s) {
	case idle:
		return std::string("idle");
	case exposing:
		return std::string("exposing");
	case exposed:
		return std::string("exposed");
	case cancelling:
		return std::string("cancelling");
	}
	throw std::runtime_error("unknown exposure state");
}

CcdState::State	CcdState::string2state(const std::string& s) {
	if (s == "idle") {
		return idle;
	}
	if (s == "exposing") {
		return exposing;
	}
	if (s == "exposed") {
		return exposed;
	}
	if (s == "cancelling") {
		return cancelling;
	}
	throw std::runtime_error("unknown exposure state");
}

} // namespace camera
} // namespace astro
