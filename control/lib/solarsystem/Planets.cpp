/*
 * Planets.cpp
 *
 * (c) Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

Mercury::Mercury() : PerturberPlanetoid("mercury",
	0.387099,					// a
	0.205634,					// e
	Angle(    48.331, Angle::Degrees),		// Omega
	Angle(     7.0048, Angle::Degrees),		// i
	Angle(    77.4552, Angle::Degrees),		// omega
	Angle(149472.6738, Angle::Degrees),		// n
	Angle(   174.7947, Angle::Degrees)) {		// M0
}

Venus::Venus() : PerturberPlanetoid("venus",
	0.723332,					// a
	0.006773,					// e
	Angle(   76.680, Angle::Degrees),		// Omega
	Angle(    3.3946, Angle::Degrees),		// i
	Angle(  131.5718, Angle::Degrees),		// omega
	Angle(58517.8149, Angle::Degrees),		// n
	Angle(   50.4071, Angle::Degrees)) {		// M0
}

Earth::Earth() : PerturberPlanetoid("earth",
	1.000000,					// a
	0.016709,					// e
	Angle(  174.876,  Angle::Degrees),		// Omega
	Angle(    0.0000, Angle::Degrees),		// i
	Angle(  102.9400, Angle::Degrees),		// omega
	Angle(35999.3720, Angle::Degrees),		// n
	Angle(  357.5256, Angle::Degrees)) {		// M0
}

Mars::Mars() : PerturberPlanetoid("mars",
	1.523692,					// a
	0.093405,					// e
	Angle(   49.557,  Angle::Degrees),		// Omega
	Angle(    1.8496, Angle::Degrees),		// i
	Angle(  336.0590, Angle::Degrees),		// omega
	Angle(19140.3023, Angle::Degrees),		// n
	Angle(   19.3879, Angle::Degrees)) {		// M0
}

Jupiter::Jupiter() : PerturberPlanetoid("jupiter",
	5.204267,					// a
	0.048775,					// e
	Angle( 100.4908, Angle::Degrees),		// Omega
	Angle(   1.3046, Angle::Degrees),		// i
	Angle(  15.5576, Angle::Degrees),		// omega
	Angle(3033.6272, Angle::Degrees),		// n
	Angle(  18.8185, Angle::Degrees)) {		// M0
}

Saturn::Saturn() : PerturberPlanetoid("saturn",
	9.582018,					// a
	0.055723,					// e
	Angle( 113.6427, Angle::Degrees),		// Omega
	Angle(   2.4852, Angle::Degrees),		// i
	Angle(  89.6567, Angle::Degrees),		// omega
	Angle(1213.8664, Angle::Degrees),		// n
	Angle( 320.3477, Angle::Degrees)) {		// M0
}

Uranus::Uranus() : PerturberPlanetoid("uranus",
	19.229412,					// a
	0.044406,					// e
	Angle( 73.9893, Angle::Degrees),		// Omega
	Angle(  0.7726, Angle::Degrees),		// i
	Angle(170.5310, Angle::Degrees),		// omega
	Angle(426.9282, Angle::Degrees),		// n
	Angle(142.9559, Angle::Degrees)) {		// M0
}

Neptune::Neptune() : PerturberPlanetoid("neptune",
	30.103658,					// a
	0.011214,					// e
	Angle(131.7942, Angle::Degrees),		// Omega
	Angle(  1.7680, Angle::Degrees),		// i
	Angle( 37.4435, Angle::Degrees),		// omega
	Angle(217.9599, Angle::Degrees),		// n
	Angle(267.7649, Angle::Degrees)) {		// M0
}

Pluto::Pluto() : PerturberPlanetoid("pluto",
	39.264230,					// a
	0.244672,					// e
	Angle(110.2867, Angle::Degrees),		// Omega
	Angle( 17.1514, Angle::Degrees),		// i
	Angle(224.0499, Angle::Degrees),		// omega
	Angle(146.3183, Angle::Degrees),		// n
	Angle( 15.0233, Angle::Degrees)) {		// M0
}

} // namespace solarsystem
} // namespace astro
