/*
 * FocusingConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

FocusState	convert(astro::focusing::Focus::state_type s) {
	switch (s) {
	case astro::focusing::Focus::IDLE:
		return FocusIDLE;
	case astro::focusing::Focus::MOVING:
		return FocusMOVING;
	case astro::focusing::Focus::MEASURING:
		return FocusMEASURING;
	case astro::focusing::Focus::FOCUSED:
		return FocusFOCUSED;
	case astro::focusing::Focus::FAILED:
		return FocusFAILED;
	}
	throw std::runtime_error("unknown focus state");
}

astro::focusing::Focus::state_type	convert(FocusState s) {
	switch (s) {
	case FocusIDLE:
		return astro::focusing::Focus::IDLE;
	case FocusMOVING:
		return astro::focusing::Focus::MOVING;
	case FocusMEASURING:
		return astro::focusing::Focus::MEASURING;
	case FocusFOCUSED:
		return astro::focusing::Focus::FOCUSED;
	case FocusFAILED:
		return astro::focusing::Focus::FAILED;
	}
	throw std::runtime_error("unknown focus state");
}

std::string	focusingstate2string(FocusState s) {
	return astro::focusing::Focus::state2string(convert(s));
}

FocusState	focusingstring2state(const std::string& s) {
	return convert(astro::focusing::Focus::string2state(s));
}

} // namespace snowstar
