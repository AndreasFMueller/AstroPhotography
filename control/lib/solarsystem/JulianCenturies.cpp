/*
 * JulianCenturies.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

/**
 * \brief Compute number of julian centuries since 2000.0
 *
 * based on Montenbruck/Pfleger p. 115
 */
double	JulianCenturies::setup(const JulianDate& jd) const {
	return (jd.T() - 2451545) / 36525;
}

} // namespace solarsystem
} // namespace astro
