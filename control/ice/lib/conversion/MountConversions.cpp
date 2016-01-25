/*
 * IceConversions.cpp -- conversions between ice and astro
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <IceConversions.h>
#include <includes.h>
#include <AstroIO.h>
#include <AstroUtils.h>

namespace snowstar {

mountstate	convert(astro::device::Mount::state_type s) {
	switch (s) {
	case astro::device::Mount::IDLE:
		return MountIDLE;
	case astro::device::Mount::ALIGNED:
		return MountALIGNED;
	case astro::device::Mount::TRACKING:
		return MountTRACKING;
	case astro::device::Mount::GOTO:
		return MountGOTO;
	}
	throw std::runtime_error("unknown state");
}

astro::device::Mount::state_type	convert(mountstate s) {
	switch (s) {
	case MountIDLE:
		return astro::device::Mount::IDLE;
	case MountALIGNED:
		return astro::device::Mount::ALIGNED;
	case MountTRACKING:
		return astro::device::Mount::TRACKING;
	case MountGOTO:
		return astro::device::Mount::GOTO;
	}
	throw std::runtime_error("unknown state");
}

mountstate	string2mountstate(const std::string& s) {
	return convert(astro::device::Mount::string2state(s));
}

std::string	state2string(mountstate s) {
	return astro::device::Mount::state2string(convert(s));
}

} // namespace snowstar
