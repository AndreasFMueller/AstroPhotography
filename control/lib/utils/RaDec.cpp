/*
 * RaDec.cpp -- Celestial coordinates implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>
#include <stdexcept>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <regex>
#include <mutex>
#include <typeinfo>

namespace astro {

RaDec::RaDec(const Vector& vector) {
	ra() = Angle(atan2(vector.y(), vector.x()));
	dec() = Angle(asin(vector.z() / vector.abs()));
}

bool	RaDec::operator<(const RaDec& other) const {
	if (dec() < other.dec()) {
		return true;
	}
	if (dec() > other.dec()) {
		return false;
	}
	return ra() < other.ra();
}

bool	RaDec::operator>=(const RaDec& other) const {
	return !(*this < other);
}

bool	RaDec::operator>(const RaDec& other) const {
	if (dec() > other.dec()) {
		return true;
	}
	if (dec() < other.dec()) {
		return false;
	}
	return ra() > other.ra();
}

bool	RaDec::operator<=(const RaDec& other) const {
	return !(*this > other);
}

std::string	RaDec::toString() const {
	return ra().hms() + " " + dec().dms();
}

const RaDec	RaDec::north_pole(Angle(0), Angle(M_PI / 2));
const RaDec	RaDec::south_pole(Angle(0), Angle(-M_PI / 2));

RaDec	RaDec::operator+(const RaDec& other) const {
	return RaDec(TwoAngles::operator+(other));
}

RaDec	RaDec::operator-(const RaDec& other) const {
	return RaDec(TwoAngles::operator-(other));
}

RaDec	RaDec::operator*(double t) const {
	return RaDec(TwoAngles::operator*(t));
}

Ecliptic	RaDec::ecliptic() const {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "converting %s to Ecliptic",
//		toString().c_str());
	double	sinbeta = sin(dec()) * cos(Angle::ecliptic_angle)
		- cos(dec()) * sin(Angle::ecliptic_angle) * sin(ra());
	Angle	beta = arcsin(sinbeta);
	double	sinlambda, coslambda;
	coslambda = cos(ra()) * cos(dec()) / cos(beta);
	sinlambda = (sin(dec()) - sin(beta) * cos(Angle::ecliptic_angle))
		/ (cos(beta) * sin(Angle::ecliptic_angle));
	Angle	lambda = arctan2(sinlambda, coslambda);
	return Ecliptic(lambda, beta);
}

void	RaDec::precess(const Precession& precession) {
	RaDec	precessed = precession(*this);
	ra() = precessed.ra();
	dec() = precessed.dec();
}

/**
 * \brief compute the coordinates 
 *
 * \param position_angle	direction measured from north
 * \param radius		length along the great circle
 */
RaDec	RaDec::exp(const Angle& position_angle, const Angle& radius) const {
	double	cosradius = cos(radius);
	double	cosdec = cos(Angle::right_angle - dec());
	double	sindec = sin(Angle::right_angle - dec());
	double	cosdec1 = cosdec * cosradius
				+ sindec * sin(radius) * cos(position_angle);
	Angle	dec1 = Angle::right_angle - arccos(cosdec1);
	double	cosbeta = (cosradius - cosdec * cosdec1)
				/ (sindec * sin(Angle::right_angle - dec1));
	double	sinbeta = sin(position_angle) * sin(radius)
			/ sin(Angle::right_angle - dec1);
	Angle	beta = arctan2(sinbeta, cosbeta);
	return RaDec(ra() + beta, dec1);
}

/**
 * \brief Compute angular distance between two points on the celestial sphere
 */
Angle	RaDec::distance(const RaDec& other) const {
	Angle	dec1 = Angle::right_angle - dec();
	Angle	dec2 = Angle::right_angle - other.dec();
	Angle	a = ra() - other.ra();
	a = a.reduced();
	double	cosd = cos(dec1) * cos(dec2) + sin(dec1) * sin(dec2) * cos(a);
	return arccos(cosd);
}

/**
 * \brief scalar product with some other vector
 */
double	RaDec::scalarproduct(const RaDec& other) const {
	return UnitVector(*this) * UnitVector(other);
}

} // namespace astro
