/*
 * Planets.cpp
 *
 * (c) Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

Mercury::Mercury() : Planetoid("mercury", 0.387099, 0.205634,
	Angle(48.331, Angle::Degrees), Angle(7.0048, Angle::Degrees),
	Angle(77.4552, Angle::Degrees), Angle(0),
	Angle(149472.6738, Angle::Degrees), Angle(174.7947, Angle::Degrees)) {
}

Venus::Venus() : Planetoid("venus", 0.723332, 0.006773,
	Angle(76.680, Angle::Degrees), Angle(3.3946, Angle::Degrees),
	Angle(131.5718, Angle::Degrees), Angle(0),
	Angle(58517.8149, Angle::Degrees), Angle(50.4071, Angle::Degrees)) {
}

Earth::Earth() : Planetoid("earth", 1.000000, 0.016709,
	Angle(174.876, Angle::Degrees), Angle(0.0000, Angle::Degrees),
	Angle(102.9400, Angle::Degrees), Angle(0),
	Angle(35999.3720, Angle::Degrees), Angle(357.5256, Angle::Degrees)) {
}

Mars::Mars() : Planetoid("mars", 1.523692, 0.093405,
	Angle(49.557, Angle::Degrees), Angle(1.8496, Angle::Degrees),
	Angle(336.0590, Angle::Degrees), Angle(0),
	Angle(19140.3023, Angle::Degrees), Angle(19.3879, Angle::Degrees)) {
}

Jupiter::Jupiter() : Planetoid("jupiter", 5.204267, 0.048775,
	Angle(100.4908, Angle::Degrees), Angle(1.3046, Angle::Degrees),
	Angle(15.5576, Angle::Degrees), Angle(0),
	Angle(3033.6272, Angle::Degrees), Angle(18.8185, Angle::Degrees)) {
}

Saturn::Saturn() : Planetoid("saturn", 9.582018, 0.055723,
	Angle(113.6427, Angle::Degrees), Angle(2.4852, Angle::Degrees),
	Angle(89.6567, Angle::Degrees), Angle(0),
	Angle(1213.8664, Angle::Degrees), Angle(320.3477, Angle::Degrees)) {
}

Uranus::Uranus() : Planetoid("uranus", 19.229412, 0.044406,
	Angle(73.9893, Angle::Degrees), Angle(0.7726, Angle::Degrees),
	Angle(170.5310, Angle::Degrees), Angle(0),
	Angle(426.9282, Angle::Degrees), Angle(142.9559, Angle::Degrees)) {
}

Neptune::Neptune() : Planetoid("neptune", 30.103658, 0.011214,
	Angle(131.7942, Angle::Degrees), Angle(1.7680, Angle::Degrees),
	Angle(37.4435, Angle::Degrees), Angle(0),
	Angle(217.9599, Angle::Degrees), Angle(267.7649, Angle::Degrees)) {
}

Pluto::Pluto() : Planetoid("pluto", 39.264230, 0.244672,
	Angle(110.2867, Angle::Degrees), Angle(17.1514, Angle::Degrees),
	Angle(224.0499, Angle::Degrees), Angle(0),
	Angle(146.3183, Angle::Degrees), Angle(15.0233, Angle::Degrees)) {
}

} // namespace solarsystem
} // namespace astro
