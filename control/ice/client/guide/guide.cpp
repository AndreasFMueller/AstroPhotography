/*
 * guide.cpp -- guide class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guide.h"
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace snowstar {
namespace app {
namespace snowguide {

ControlType	Guide::string2type(const std::string& type) {
	if (type == "GP") {
		return ControlGuiderPort;
	}
	if (type == "AO") {
		return ControlAdaptiveOptics;
	}
	std::string	cause = astro::stringprintf("unknown type %s",
				type.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw std::runtime_error(cause);
}

} // namespace snowguide
} // namespace app
} // namespace snowstar
