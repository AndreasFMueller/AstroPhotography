/*
 * EclipticalCoordinates.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>
#include <sstream>
#include <iomanip>

namespace astro {
namespace solarsystem {

EclipticalCoordinates::EclipticalCoordinates() : _l(0.), _r(0.), _b(0.) {
}

EclipticalCoordinates::EclipticalCoordinates(const Angle& l, double r,
		const Angle& b) : _l(l), _r(r), _b(b) {
	_l.reduce(0);
}

EclipticalCoordinates	EclipticalCoordinates::operator+(
	const EclipticalCoordinates& other) const {
	return EclipticalCoordinates(_l + other._l, _r + other._r,
		_b + other._b);
}

EclipticalCoordinates	EclipticalCoordinates::operator-(
	const EclipticalCoordinates& other) const {
	return EclipticalCoordinates(_l - other._l, _r - other._r,
		_b - other._b);
}

EclipticalCoordinates	EclipticalCoordinates::operator*(double l) const {
	return EclipticalCoordinates(l * _l, l * _r, l * _b);
}

EclipticalCoordinates	operator*(double l, const EclipticalCoordinates& ecl) {
	return ecl * l;
}

Vector	EclipticalCoordinates::v() const {
	double	c = cos(_b);
	return Vector(_r * cos(_l) * c, _r * sin(_l) * c, _r * sin(_b));
}

std::string	EclipticalCoordinates::toString() const {
	std::ostringstream	out;
	out << _l.dms() << " ";
	out << std::setw(10) << std::setprecision(6) << _r << " ";
	out << _b.dms();
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const EclipticalCoordinates& ec) {
	out << ec.toString();
	return out;
}

} // namespace solarsystem
} // namespace astro
