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

std::string	TwoAngles::toString() const {
	std::ostringstream	out;
	out << _a1.degrees() << "/" << _a2.degrees();
	return out.str();
}

} // namespace astro
