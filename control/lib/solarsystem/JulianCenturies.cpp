/*
 * JulianCenturies.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>
#include <AstroDebug.h>

namespace astro {
namespace solarsystem {

/**
 * \brief Compute number of julian centuries since 2000.0
 *
 * based on Montenbruck/Pfleger p. 115
 */
double	JulianCenturies::setup(const JulianDate& jd) const {
	double	t = jd.years();
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "T = %f", t);
	return t;
}

} // namespace solarsystem
} // namespace astro
