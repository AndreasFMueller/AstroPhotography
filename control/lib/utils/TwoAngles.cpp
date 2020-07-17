/*
 * TwoAngles.cpp -- angle pair implementation
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
#include <sstream>

namespace astro {

Angle&	TwoAngles::operator[](int i) {
	switch (i) {
	case 0:	return a1();
	case 1: return a2();
	}
	throw std::range_error("angle index out of range");
}

const Angle&	TwoAngles::operator[](int i) const {
	switch (i) {
	case 0:	return a1();
	case 1: return a2();
	}
	throw std::range_error("angle index out of range");
}

TwoAngles	TwoAngles::operator+(const TwoAngles& other) const {
	return TwoAngles(_a1 + other._a1, _a2 + other._a2);
}

TwoAngles	TwoAngles::operator-(const TwoAngles& other) const {
	return TwoAngles(_a1 - other._a1, _a2 - other._a2);
}

TwoAngles	TwoAngles::operator*(double t) const {
	return TwoAngles(_a1 * t, _a2 * t);
}

bool	TwoAngles::operator==(const TwoAngles& other) const {
	return (_a1 == other._a1) && (_a2 == other._a2);
}

bool	TwoAngles::operator!=(const TwoAngles& other) const {
	return (_a1 != other._a1) || (_a2 != other._a2);
}

std::string	TwoAngles::toString(Angle::unit unit) const {
	std::ostringstream	out;
	switch (unit) {
	case Angle::Radians:
		out << _a1.radians() << "/" << _a2.radians();
		break;
	case Angle::Degrees:
		out << _a1.degrees() << "/" << _a2.degrees();
		break;
	case Angle::Hours:
		out << _a1.hours() << "/" << _a2.hours();
		break;
	case Angle::Minutes:
		out << _a1.minutes() << "/" << _a2.minutes();
		break;
	case Angle::Seconds:
		out << _a1.seconds() << "/" << _a2.seconds();
		break;
	case Angle::ArcMinutes:
		out << _a1.arcminutes() << "/" << _a2.arcminutes();
		break;
	case Angle::ArcSeconds:
		out << _a1.arcseconds() << "/" << _a2.arcseconds();
		break;
	case Angle::Revolutions:
		out << _a1.revolutions() << "/" << _a2.revolutions();
		break;
	}
	return out.str();
}

bool	TwoAngles::operator<(const TwoAngles& other) const {
	if (a1() < other.a1()) {
		return true;
	}
	if (a1() > other.a1()) {
		return false;
	}
	return a2() < other.a2();
}

std::ostream&	operator<<(std::ostream& out, const TwoAngles& angles) {
	out << angles.toString();
	return out;
}

} // namespace astro
