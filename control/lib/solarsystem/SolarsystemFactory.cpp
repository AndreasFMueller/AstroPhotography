/*
 * SolarsystemFactory.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#include <AstroSolarsystem.h>
#include <AstroDebug.h>

namespace astro {
namespace solarsystem {

SolarsystemBodyPtr	SolarsystemFactory::get(const std::string& name) {
	SolarsystemBody	*result = NULL;
	if (name == "sun") {
		result = new Sun();
	}
	if (name == "moon") {
		result = new Moon();
	}
	if (NULL != result) {
		return SolarsystemBodyPtr(result);
	}
	std::string	msg = stringprintf("Solarsystem body '%s' unknown",
		name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace solarsystem
} // namespace astro
