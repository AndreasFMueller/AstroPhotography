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

} // namespace astro
