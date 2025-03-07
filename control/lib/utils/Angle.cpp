/*
 * Angle.cpp -- Angle class implementation
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

static std::string	xms(double value, const char separator, int precision) {
	int	sign = (value >= 0) ? 1 : -1;
	value = fabs(value);
	int	X = floor(value);
	value = 60 * (value - X);
	int	M = floor(value);
	double	S = 60 * (value - M);
	if (precision < 0) {
		return stringprintf("%c%02d%c%02d",
			(sign < 0) ? '-' : '+', X, separator, M);
	} else {
		// round S to <precision> decimal places
		double	f = pow(10., precision);
		S = round(f * S) / f;
		if (S >= 60) {
			S -= 60;
			M += 1;
			if (M >= 60) {
				X += 1;
			}
		}
		return stringprintf("%c%02d%c%02d%c%0*.*f",
			(sign < 0) ? '-' : '+',
			X, separator, M, separator,
			((precision > 0) ? 3 : 2) + precision, precision, S);
	}
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

Angle::Angle(double angle, unit u) : _angle(angle) {
	switch (u) {
	case Radians:
		_angle = angle;
		return;
	case Degrees:
		degrees(angle);
		return;
	case ArcMinutes:
		arcminutes(angle);
		return;
	case ArcSeconds:
		arcseconds(angle);
		return;
	case Hours:
		hours(angle);
		return;
	case Minutes:
		minutes(angle);
		return;
	case Seconds:
		seconds(angle);
		return;
	case Revolutions:
		revolutions(angle);
		return;
	}
}

Angle::Angle(double x, double y) {
	_angle = atan2(y, x);
}

Angle::Angle(const std::string& a, unit u) {
	switch (u) {
	case Radians:
		_angle = std::stod(a);
		return;
	case Degrees:
		_angle = dms_to_angle(a).radians();
		return;
	case Hours:
		_angle = hms_to_angle(a).radians();
		return;
	case Revolutions:
		revolutions(std::stod(a));
		return;
	case ArcMinutes:
		arcminutes(std::stod(a));
		return;
	case ArcSeconds:
		arcseconds(std::stod(a));
		return;
	case Minutes:
		minutes(std::stod(a));
		return;
	case Seconds:
		minutes(std::stod(a));
		return;
	}
	throw std::runtime_error("unknown unit");
}

double	Angle::degrees() const {
	return radians_to_degrees(_angle);
}

double	Angle::arcminutes() const {
	return 60 * degrees();
}

double	Angle::arcseconds() const {
	return 3600 * degrees();
}

void	Angle::degrees(double degrees) {
	_angle = degrees_to_radians(degrees);
}

void	Angle::arcminutes(double arcminutes) {
	_angle = degrees_to_radians(arcminutes / 60.);
}

void	Angle::arcseconds(double arcseconds) {
	_angle = degrees_to_radians(arcseconds / 3600.);
}

std::string	Angle::dms(const char separator, int precision) const {
	return xms(degrees(), separator, precision);
}

double	Angle::hours() const {
	return radians_to_hours(_angle);
}

void	Angle::hours(double hours) {
	_angle = hours_to_radians(hours);
}

double	Angle::minutes() const {
	return 60. * hours();
}

double	Angle::seconds() const {
	return 3600. * hours();
}

void	Angle::minutes(double minutes) {
	hours(minutes / 60.);
}

void	Angle::seconds(double seconds) {
	hours(seconds / 3600.);
}

std::string	Angle::hms(const char separator, int precision) const {
	return xms(hours(), separator, precision);
}

double	Angle::revolutions() const {
	return _angle / (2 * M_PI);
}

void	Angle::revolutions(double revolutions) {
	radians(2 * M_PI * revolutions);
}

double	Angle::value(Angle::unit u) const {
	switch (u) {
	case Radians:
		return radians();
	case Degrees:
		return degrees();
	case Hours:
		return hours();
	case Minutes:
		return minutes();
	case Seconds:
		return seconds();
	case Revolutions:
		return revolutions();
	case ArcMinutes:
		return arcminutes();
	case ArcSeconds:
		return arcseconds();
	}
	throw std::runtime_error("unknown unit");
}

Angle	Angle::operator+(const Angle& other) const {
	return Angle(_angle + other._angle);
}

Angle	Angle::operator-(const Angle& other) const {
	return Angle(_angle - other._angle);
}

Angle	Angle::operator-() const {
	return Angle(-_angle);
}

Angle	Angle::operator*(const double& other) const {
	return Angle(_angle * other);
}

double	Angle::operator/(const Angle& other) const {
	return _angle / other._angle;
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

double	Angle::cos() const {
	return ::cos(_angle);
}

double	Angle::sin() const {
	return ::sin(_angle);
}

double	Angle::tan() const {
	return ::tan(_angle);
}

double	cos(const Angle& a) { return ::cos(a.radians()); }
double	sin(const Angle& a) { return ::sin(a.radians()); }
double	tan(const Angle& a) { return ::tan(a.radians()); }
double	cot(const Angle& a) { return 1. / tan(a); }
double	sec(const Angle& a) { return 1 / cos(a); }
double	csc(const Angle& a) { return 1 / sin(a); }

Angle	arccos(double x) {
	return Angle(acos(x));
}

Angle	arcsin(double x) {
	return Angle(asin(x));
}

Angle	arctan(double x) {
	return Angle(atan(x));
}

Angle	arctan2(double y, double x) {
	return Angle(atan2(y, x));
}

Angle	operator*(double l, const Angle& a) {
	return a * l;
}


class angle_parser {
public:
	static std::string	r;
	double	_value;
	angle_parser(const std::string& xms);
	double	value() const { return _value; }
	int	integer(const std::smatch& matches, int i);
	double	fraction(const std::smatch& matches, int i);
	int	sign(const std::smatch& matches, int i);
};

//                               1      2       34           5 6       78           9 1       1
//                                                                                    0       1
std::string	angle_parser::r("([-+])?([0-9]*)((\\.[0-9]*)|(:([0-9]*)((\\.[0-9]*)|(:([0-9]*)(\\.[0-9]*)?))?))?");

int	angle_parser::integer(const std::smatch& matches, int i) {
	if (matches.position(i) < 0) {
		return 0;
	}
	if (matches.length(i) == 0) {
		return 0;
	}
	return std::stoi(matches[i]);
}

double	angle_parser::fraction(const std::smatch& matches, int i) {
	if (matches.position(i) < 0) {
		return 0.;
	}
	if (matches.length(i) == 0) {
		return 0.;
	}
	return std::stod(matches[i]);
}

int	angle_parser::sign(const std::smatch& matches, int i) {
	if (matches.position(i) < 0) {
		return 1;
	}
	return (matches[i] == "-") ? -1 : 1;
}

angle_parser::angle_parser(const std::string& xms) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parse angle spec: %s", xms.c_str());
	std::regex	regex;
	try {
		regex = std::regex(r, std::regex::extended);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "regex exception: %s %s",
			typeid(x).name(), x.what());
		throw;
	}
	std::smatch	matches;
	if (!regex_match(xms, matches, regex)) {
		std::string	msg = stringprintf("bad angle spec '%s'",
			xms.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
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

const Angle Angle::right_angle(M_PI/2);

const Angle Angle::ecliptic_angle(23.43 * M_PI / 180);

Angle	Angle::ecliptic(double T) {
	// JPL formular (see https://en.wikipedia.org/wiki/Ecliptic)
	double	a = 23.4392794 + (-0.0130102136
			+ (-0.00000005086 + 0.000000556 * T) * T) * T;
	return Angle(a * M_PI / 180);
}

Angle	abs(const Angle& a) {
	return Angle(fabs(a.radians()));
}

std::ostream&	operator<<(std::ostream& out, const Angle& angle) {
	out << angle.dms();
	return out;
}

} // namespace astro
