/*
 * Moon.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

using astro::Angle;

namespace astro {
namespace solarsystem {

Moon::Moon() : SolarsystemBody("moon") {
}

RaDec	Moon::ephemerisT(double T0) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "compute moon ephemeris for T0 = %f",
		T0);
	Angle	L0(0.606433 + 1336.855225 * T0, Angle::Revolutions);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "L0 = %f", L0.revolutions());
	Angle	l( 0.374897 + 1325.855410 * T0, Angle::Revolutions);
	Angle	ls(0.993133 +   99.997361 * T0, Angle::Revolutions);
	Angle	D( 0.827361 + 1236.853086 * T0, Angle::Revolutions);
	Angle	F( 0.259086 + 1342.227825 * T0, Angle::Revolutions);

	Angle	dL(	 22640. * sin(l)
			- 4586. * sin(l - 2. * D)
			+ 2370. * sin(2. * D)
			+  769. * sin(2. * l)
			-  668. * sin(ls)
			-  412. * sin(2. * F)
			-  212. * sin(2. * l - 2. * D)
			-  206. * sin(l + ls - 2. * D)
			+  192. * sin(l + 2. * D)
			-  165. * sin(ls - 2. * D)
			-  125. * sin(D)
			-  110. * sin(l + ls)
			+  148. * sin(l - ls)
			-   55. * sin(2. * F - 2. * D), Angle::ArcSeconds);
	Angle	l_Moon = L0 + dL;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "l_Moon = %f", l_Moon.hours());

	Angle	S = F + dL + Angle(412. * sin(2. * F) + 541. * sin(ls),
				Angle::ArcSeconds);
	Angle	h = F - 2. * D;
	Angle	N(	- 526. * sin(h)
			+  44. * sin(l + h)
			-  31. * sin(-l + h)
			-  23. * sin(ls + h)
			+  11. * sin(-ls + h)
			-  25. * sin(-2. * l + F)
			+  21. * sin(-l + F), Angle::ArcSeconds);
	Angle	b_Moon = Angle(18520.0 * sin(S), Angle::ArcSeconds) + N;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "b_Moon = %f", b_Moon.revolutions());

	UnitVector	direction(SphericalCoordinates(l_Moon,
				Angle::right_angle - b_Moon));
	Angle	eps(23.43929111, Angle::Degrees);
	Rotation3D	Rx('x', -eps);
	RaDec	result(Rx(direction));
	result.a1().reduce(0);
	return result;
}

} // namespace solarsystem
} // namespace astro
