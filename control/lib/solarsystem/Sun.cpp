/*
 * Sun.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

Sun::Sun() : SolarsystemBody("sun") {
}

RaDec	Sun::ephemerisT(double T0) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute sun ephemeris for T0 = %f", T0);
	Angle	eps(23.4392911, Angle::Degrees);
	Rotation3D	Rx('x', -eps);
	Angle	M(0.993133 + 99.997361 * T0, Angle::Revolutions);
	Angle	L = Angle(0.7859453, Angle::Revolutions) + M +
		Angle(6893.0 * sin(M) + 72.0 * sin(2. * M) + 6191.2 * T0,
			Angle::ArcSeconds);
	SphericalCoordinates	spherical(L, Angle::right_angle);
	UnitVector	pos(spherical);
	RaDec	result(Rx(pos));
	result.a1().reduce(0);
	return result;
}

} // namespace solarsystem
} // namespace astro
