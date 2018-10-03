/*
 * Ecliptic.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>

namespace astro {

std::string	Ecliptic::toString() const {
	return stringprintf("lambda=%.4f,beta=%.4f", lambda().degrees(),
		beta().degrees());
}

Ecliptic::Ecliptic(const RaDec& radec) {
	Ecliptic	e = radec.ecliptic();
	lambda() = e.lambda();
	beta() = e.beta();
}

RaDec	Ecliptic::radec() const {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting %s to RaDec",
//		toString().c_str());
	double	sindelta = sin(beta()) * cos(Angle::ecliptic_angle)
		+ cos(beta()) * sin(Angle::ecliptic_angle) * sin(lambda());
	Angle	delta = arcsin(sindelta);
	double	cosalpha, sinalpha;
	cosalpha = cos(lambda()) * cos(beta()) / cos(delta);
	sinalpha = -(sin(beta()) - sindelta * cos(Angle::ecliptic_angle))
			/ (sin(Angle::ecliptic_angle) * cos(delta));
	Angle	alpha = arctan2(sinalpha, cosalpha);
	return RaDec(alpha, delta);
}

void	Ecliptic::precess(const Precession& precession) {
	Ecliptic	precessed = precession(*this);
	lambda() = precessed.lambda();
	beta() = precessed.beta();
}

} // namespace astro
