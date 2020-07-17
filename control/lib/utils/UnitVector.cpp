/*
 * UnitVector.cpp -- coordinate system implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>
#include <stdexcept>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {

UnitVector::UnitVector() {
	_x[0] = 1.;
	_x[1] = 0.;
	_x[2] = 0.;
}

UnitVector::UnitVector(const RaDec& radec) {
	_x[2] = sin(radec.dec());
	double	r = cos(radec.dec());
	_x[0] = r * cos(radec.ra());
	_x[1] = r * sin(radec.ra());
}

UnitVector::UnitVector(const SphericalCoordinates& spherical) {
	_x[2] = cos(spherical.theta());
	double	r = sin(spherical.theta());
	_x[0] = r * cos(spherical.phi());
	_x[1] = r * sin(spherical.phi());
}

UnitVector::UnitVector(const Vector& v) {
	*this = v.normalized();
}

UnitVector::UnitVector(const double x[3]) : Vector(x) {
	*this = normalized();
}

UnitVector&	UnitVector::operator=(const Vector& other) {
	Vector	u = other.normalized();
	_x[0] = u.x();
	_x[1] = u.y();
	_x[2] = u.z();
	return *this;
}

Angle	UnitVector::angle(const UnitVector& other) const {
	return Angle(acos(*this * other));
}

Vector	UnitVector::operator()(const Vector& other) const {
	return (*this) * ((*this) * other);
}

UnitVector	UnitVector::Ex() {
	double	x[3] = { 1., 0., 0. };
	return UnitVector(x);
}

UnitVector	UnitVector::Ey() {
	double	x[3] = { 0., 1., 0. };
	return UnitVector(x);
}

UnitVector	UnitVector::Ez() {
	double	x[3] = { 0., 0., 1. };
	return UnitVector(x);
}

} // namespace astro
