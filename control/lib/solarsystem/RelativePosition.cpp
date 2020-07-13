/*
 * RelativePosition.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

Angle   RelativePosition::eps(23.4392911, Angle::Degrees);

/**
 * \brief Compute the RaDec 
 *
 * \param v	direction to the object
 */
RaDec	RelativePosition::radec(const Vector& v) const {
        Rotation3D      Rx('x', -eps);
	Vector	r = v - _earth.v();
        RaDec   result(Rx(r));
        result.a1().reduce(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "radec of %s: %s", r.toString().c_str(),
		result.toString().c_str());
        return result;
}

/**
 * \brief Compute the position of a planet
 *
 * \param planet	the planet to compute the position from
 */
RaDec	RelativePosition::radec(Planetoid& planet) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute position of %s", planet.name().c_str());
	Vector	r = planet.XYZ(_T);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "relative position of %s: %s",
		planet.name().c_str(), r.toString().c_str());
	return radec(r);
}

RaDec	RelativePosition::radec(PerturbedPlanetoid *planet) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute position of %s",
		planet->name().c_str());
	Vector	r = planet->XYZ(_T);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "relative position of %s: %s",
		planet->name().c_str(), r.toString().c_str());
	return radec(r);
}

} // namespace solarsystem
} // namespace astro
