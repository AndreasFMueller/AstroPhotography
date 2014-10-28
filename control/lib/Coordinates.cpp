/*
 * Coordinates.cpp -- coordinate system implementation
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
#include <astroregex.h>

namespace astro {

static std::string	xms(double value, const char separator) {
	int	sign = (value >= 0) ? 1 : -1;
	value = fabs(value);
	int	X = floor(value);
	value = 60 * (value - X);
	int	M = floor(value);
	double	S = 60 * (value - M);
	return stringprintf("%c%02d%c%02d%c%06.3f", (sign < 0) ? '-' : '+',
		X, separator, M, separator, S);
}

static double	angle_reduction(const double a, const double base) {
	double	ab = a - base;
	return base + ab - 2 * M_PI * floor(ab / (2 * M_PI));
}

void	Angle::reduce(double base) {
	_angle = angle_reduction(_angle, base);
}

Angle	Angle::reduced(const double base) const {
	return Angle(angle_reduction(_angle, base));
}

double	Angle::degrees_to_radians(const double d) {
	return M_PI * d / 180.;
}

double	Angle::hours_to_radians(const double h) {
	return M_PI * h / 12.;
}

double	Angle::radians_to_hours(const double r) {
	return 12. * r / M_PI;
}

double	Angle::radians_to_degrees(const double r) {
	return 180. * r / M_PI;
}

Angle::Angle(double angle) : _angle(angle) {
	//reduce();
}

double	Angle::degrees() const {
	return radians_to_degrees(_angle);
}

void	Angle::degrees(double degrees) {
	_angle = degrees_to_radians(degrees);
}

std::string	Angle::dms(const char separator) const {
	return xms(degrees(), separator);
}

double	Angle::hours() const {
	return radians_to_hours(_angle);
}

void	Angle::hours(double hours) {
	_angle = hours_to_radians(hours);
}

std::string	Angle::hms(const char separator) const {
	return xms(hours(), separator);
}

Angle	Angle::operator+(const Angle& other) const {
	return Angle(_angle + other._angle);
}

Angle	Angle::operator-(const Angle& other) const {
	return Angle(_angle - other._angle);
}

Angle	Angle::operator*(const double& other) const {
	return Angle(_angle * other);
}

bool	Angle::operator<(const Angle& other) const {
	return _angle < other._angle;
}

bool	Angle::operator<=(const Angle& other) const {
	return _angle <= other._angle;
}

bool	Angle::operator>(const Angle& other) const {
	return _angle > other._angle;
}

bool	Angle::operator>=(const Angle& other) const {
	return _angle >= other._angle;
}

bool	Angle::operator==(const Angle& other) const {
	return _angle == angle_reduction(other._angle, _angle);
}

bool	Angle::operator!=(const Angle& other) const {
	return !((*this) == other);
}

double	cos(const Angle& a) { return ::cos(a.radians()); }
double	sin(const Angle& a) { return ::sin(a.radians()); }
double	tan(const Angle& a) { return ::tan(a.radians()); }
double	cot(const Angle& a) { return 1. / tan(a); }
double	sec(const Angle& a) { return 1 / cos(a); }
double	csc(const Angle& a) { return 1 / sin(a); }

class angle_parser {
public:
	static std::string	r;
	double	_value;
	angle_parser(const std::string& xms);
	double	value() const { return _value; }
	int	integer(const astro::smatch& matches, int i);
	double	fraction(const astro::smatch& matches, int i);
	int	sign(const astro::smatch& matches, int i);
};

//                               1      2       34           5 6       78           9 1       1
//                                                                                    0       1
std::string	angle_parser::r("([-+])?([0-9]*)((\\.[0-9]*)|(:([0-9]*)((\\.[0-9]*)|(:([0-9]*)(\\.[0-9]*)?))?))?");

int	angle_parser::integer(const astro::smatch& matches, int i) {
	if (matches.position(i) < 0) {
		return 0;
	}
	if (matches.length(i) == 0) {
		return 0;
	}
	return std::stoi(matches[i]);
}

double	angle_parser::fraction(const astro::smatch& matches, int i) {
	if (matches.position(i) < 0) {
		return 0.;
	}
	if (matches.length(i) == 0) {
		return 0.;
	}
	return std::stod(matches[i]);
}

int	angle_parser::sign(const astro::smatch& matches, int i) {
	if (matches.position(i) < 0) {
		return 1;
	}
	return (matches[i] == "-") ? -1 : 1;
}

angle_parser::angle_parser(const std::string& xms) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parse angle spec: %s", xms.c_str());
	astro::regex	regex;
	try {
		regex = astro::regex(r, astro::regex::extended);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "regex exception: %s %s",
			typeid(x).name(), x.what());
		throw;
	}
	astro::smatch	matches;
	if (!regex_match(xms, matches, regex)) {
		std::string	msg = stringprintf("bad angle spec '%s'",
			xms.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	for (int i = 0; i < 12; i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "matches[%d]: %d %d '%s'",
			i, matches.position(i), matches.length(i),
			std::string(matches[i]).c_str());
	}

	// initialization
	_value = 0;

	// handle the case of decimal number
	_value += integer(matches, 2);
	_value += fraction(matches, 4);

	// separate minutes field
	_value += integer(matches, 6) / 60.;

	// with fractional part
	_value += fraction(matches, 8) / 60.;

	// separate seconds field
	_value += integer(matches, 10) / 3600.;
	_value += fraction(matches, 11) / 3600.;

	// add sign
	_value *= sign(matches, 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsed value: %s -> %f", xms.c_str(),
		_value);
}

Angle	Angle::hms_to_angle(const std::string& hms) {
	Angle	result;
	result.hours(angle_parser(hms).value());
	return result;
}

Angle	Angle::dms_to_angle(const std::string& dms) {
	Angle	result;
	result.degrees(angle_parser(dms).value());
	return result;
}


//////////////////////////////////////////////////////////////////////
// TwoAngles implementation
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// Comparison operators for RaDec
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// SphericalCoordinates implementation
//////////////////////////////////////////////////////////////////////
SphericalCoordinates::SphericalCoordinates(const LongLat& longlat)
	: TwoAngles(longlat.longitude(), Angle(M_PI / 2) - longlat.latitude()) {
}

SphericalCoordinates::SphericalCoordinates(const RaDec& radec)
	: TwoAngles(radec.ra(), Angle(M_PI / 2) - radec.dec()) {
}

Angle	operator-(const SphericalCoordinates& s1, const SphericalCoordinates& s2) {
	return UnitVector(s1).angle(UnitVector(s2));
}

} // namespace astro
