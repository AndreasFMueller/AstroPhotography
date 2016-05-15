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

FocusState	convert(astro::focusing::Focusing::state_type s) {
	switch (s) {
	case astro::focusing::Focusing::IDLE:
		return FocusIDLE;
	case astro::focusing::Focusing::MOVING:
		return FocusMOVING;
	case astro::focusing::Focusing::MEASURING:
		return FocusMEASURING;
	case astro::focusing::Focusing::FOCUSED:
		return FocusFOCUSED;
	case astro::focusing::Focusing::FAILED:
		return FocusFAILED;
	}
	throw std::runtime_error("unknown focus state");
}

astro::focusing::Focusing::state_type	convert(FocusState s) {
	switch (s) {
	case FocusIDLE:
		return astro::focusing::Focusing::IDLE;
	case FocusMOVING:
		return astro::focusing::Focusing::MOVING;
	case FocusMEASURING:
		return astro::focusing::Focusing::MEASURING;
	case FocusFOCUSED:
		return astro::focusing::Focusing::FOCUSED;
	case FocusFAILED:
		return astro::focusing::Focusing::FAILED;
	}
	throw std::runtime_error("unknown focus state");
}

std::string	focusingstate2string(FocusState s) {
	return astro::focusing::Focusing::state2string(convert(s));
}

FocusState	focusingstring2state(const std::string& s) {
	return convert(astro::focusing::Focusing::string2state(s));
}

FocusMethod	convert(astro::focusing::Focusing::method_type m) {
	switch (m) {
	case astro::focusing::Focusing::BRENNER:
		return FocusBrenner;
	case astro::focusing::Focusing::FWHM:
		return FocusFWHM;
	case astro::focusing::Focusing::MEASURE:
		return FocusMEASURE;
	}
	throw std::runtime_error("unknown focus method");
}

astro::focusing::Focusing::method_type	convert(FocusMethod m) {
	switch (m) {
	case FocusBrenner:
		return astro::focusing::Focusing::BRENNER;
	case FocusFWHM:
		return astro::focusing::Focusing::FWHM;
	case FocusMEASURE:
		return astro::focusing::Focusing::MEASURE;
	}
	throw std::runtime_error("unknown focus method");
}

std::string     focusingmethod2string(FocusMethod m) {
	return astro::focusing::Focusing::method2string(convert(m));
}

FocusMethod     focusingstring2method(const std::string& m) {
	return convert(astro::focusing::Focusing::string2method(m));
}

} // namespace snowstar
