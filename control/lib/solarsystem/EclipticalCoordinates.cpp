/*
 * EclipticalCoordinates.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

EclipticalCoordinates::EclipticalCoordinates() : _l(0.), _r(0.), _b(0.) {
}

EclipticalCoordinates::EclipticalCoordinates(const Angle& l, double r,
		const Angle& b) : _l(l), _r(r), _b(b) {
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

} // namespace solarsystem
} // namespace astro
