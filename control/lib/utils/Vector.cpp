/*
 * Vector.cpp -- coordinate system implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCoordinates.h>
#include <cmath>
#include <stdexcept>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {

//////////////////////////////////////////////////////////////////////
// Vector implementation
//////////////////////////////////////////////////////////////////////
Vector::Vector() {
	_x[0] = 0.;
	_x[1] = 0.;
	_x[2] = 0.;
}

Vector::Vector(const double x[3]) {
	_x[0] = x[0];
	_x[1] = x[1];
	_x[2] = x[2];
}

double	Vector::abs() const {
	return sqrt(_x[0] * _x[0] + _x[1] * _x[1] + _x[2] * _x[2]);
}

double	Vector::operator*(const Vector& other) const {
	return _x[0] * other._x[0] + _x[1] * other._x[1] + _x[2] * other._x[2];
}

Vector	Vector::cross(const Vector& other) const {
	Vector	result;
	result._x[0] = _x[1] * other._x[2] - _x[2] * other._x[1];
	result._x[1] = _x[2] * other._x[0] - _x[0] * other._x[2];
	result._x[2] = _x[0] * other._x[1] - _x[1] * other._x[0];
	return result;
}

Vector	Vector::operator-() const {
	Vector	result;
	result._x[0] = -_x[0];
	result._x[1] = -_x[1];
	result._x[2] = -_x[2];
	return result;
}

Vector	Vector::operator+(const Vector& other) const {
	Vector	result;
	result._x[0] = _x[0] + other._x[0];
	result._x[1] = _x[1] + other._x[1];
	result._x[2] = _x[2] + other._x[2];
	return result;
}

Vector	Vector::operator-(const Vector& other) const {
	Vector	result;
	result._x[0] = _x[0] - other._x[0];
	result._x[1] = _x[1] - other._x[1];
	result._x[2] = _x[2] - other._x[2];
	return result;
}

Vector	Vector::operator*(double l) const {
	Vector	result;
	result._x[0] = l * _x[0];
	result._x[1] = l * _x[1];
	result._x[2] = l * _x[2];
	return result;
}

std::string	Vector::toString() const {
	return stringprintf("(%f,%f,%f)", _x[0], _x[1], _x[2]);
}

Vector	Vector::normalized() const {
	return (*this) * (1. / abs());
}

Vector	Vector::Ex() {
	double	x[3] = { 1., 0., 0. };
	return Vector(x);
}

Vector	Vector::Ey() {
	double	x[3] = { 0., 1., 0. };
	return Vector(x);
}

Vector	Vector::Ez() {
	double	x[3] = { 0., 0., 1. };
	return Vector(x);
}

//////////////////////////////////////////////////////////////////////
// UnitVector implementation
//////////////////////////////////////////////////////////////////////
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
