/*
 * SolarsystemBody.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

SolarsystemBody::SolarsystemBody(const std::string& name) : _name(name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating solar system body named '%s'",
		_name.c_str());
}

RaDec	SolarsystemBody::ephemeris(time_t when) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ephemeris for time %ld", when);
	JulianDate	jd(when);
	return ephemeris(jd);
}

RaDec	SolarsystemBody::ephemeris(const JulianDate& when) {
	double	T0 = (when.T() - 2451545.) / 36525.;
	return this->ephemerisT(T0);
}

} // namespace solarsystem 
} // namespace astro
