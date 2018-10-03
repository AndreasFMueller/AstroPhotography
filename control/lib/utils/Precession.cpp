/*
 * Precession.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <AstroDebug.h>

namespace astro {

void	Precession::setup(time_t when) {
	JulianDate	jd(when);
	precessionangle = jd.years() * Angle((50.35 / 3600) * (M_PI / 180));
}

Precession::Precession() {
	time_t	when;
	time(&when);
	setup(when);
}

Precession::Precession(double years) {
	precessionangle = years * Angle((50.35 / 3600) * (M_PI / 180));
}

Precession::Precession(time_t when) {
	setup(when);
}

Ecliptic	Precession::operator()(const Ecliptic& ecliptic) const {
	return Ecliptic(ecliptic.lambda() + precessionangle, ecliptic.beta());
}

RaDec	Precession::operator()(const RaDec& radec) const {
	RaDec	result = ((*this)(radec.ecliptic())).radec();
	if (result.ra() < 0) {
		result.ra() = result.ra() + Angle(2 * M_PI);
	}
	return result;
}

} // namespace astro
