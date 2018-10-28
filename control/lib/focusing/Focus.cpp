/*
 * Focus.cpp -- commmon stuff
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief String to state conversion
 */
Focus::state_type Focus::string2state(const std::string& s) {
	if (s == "idle") {
		return Focus::IDLE;
	}
	if (s == "moving") {
		return Focus::MOVING;
	}
	if (s == "measuring") {
		return Focus::MEASURING;
	}
	if (s == "measured") {
		return Focus::MEASURED;
	}
	if (s == "focused") {
		return Focus::FOCUSED;
	}
	if (s == "failed") {
		return Focus::FAILED;
	}
	throw std::runtime_error("bad focus status");
}

/**
 * \brief State to string conversion
 */
std::string	Focus::state2string(state_type s) {
	switch (s) {
	case IDLE:
		return std::string("idle");
		break;
	case MOVING:
		return std::string("moving");
		break;
	case MEASURING:
		return std::string("measuring");
		break;
	case MEASURED:
		return std::string("measured");
		break;
	case FOCUSED:
		return std::string("focused");
		break;
	case FAILED:
		return std::string("failed");
		break;
	}
	throw std::runtime_error("bad focus status");
}

} // namespace focusing
} // namespace astro
