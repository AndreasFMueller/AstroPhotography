/*
 * ControlDeviceType.cpp -- implementation of conversion functions
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

std::string	type2string(ControlDeviceType caltype) {
	switch (caltype) {
	case GP:
		return std::string("GuidePort");
	case AO:
		return std::string("AdaptiveOptics");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "unknown calibration type %d", caltype);
	throw std::runtime_error("unknown calibration type");
}

ControlDeviceType	string2type(const std::string& calname) {
	if (calname == std::string("GuidePort")) {
		return GP;
	}
	if (calname == std::string("GP")) {
		return GP;
	}
	if (calname == std::string("AdaptiveOptics")) {
		return AO;
	}
	if (calname == std::string("AO")) {
		return AO;
	}
	std::string	msg = stringprintf("unknown calibration type: %s",
		calname.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace guiding
} // namespace astro
