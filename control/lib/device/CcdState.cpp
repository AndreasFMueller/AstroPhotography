/*
 * CcdState.cpp -- Implementation of CCD state
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>

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
	case streaming:
		return std::string("streaming");
	}
	std::string	msg = stringprintf("bad exposure state: %d", (int)s);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
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
	if (s == "streaming") {
		return streaming;
	}
	std::string	msg = stringprintf("unknown exposure state '%s'",
		s.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace camera
} // namespace astro
