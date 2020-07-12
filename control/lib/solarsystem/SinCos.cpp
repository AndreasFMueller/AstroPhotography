/*
 * SinCos.cpp -- Auxiliary class to compute values of sin() and cos()
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroSolarsystem.h>

namespace astro {
namespace solarsystem {

SinCos::SinCos(double cos, double sin) : Angle(cos, sin), _cos(cos), _sin(sin) {
}

SinCos::SinCos() : _cos(1), _sin(0) {
}

SinCos::SinCos(const Angle& angle)
	: Angle(angle), _cos(astro::cos(angle)), _sin(astro::sin(angle)) {
}

SinCos::SinCos(const std::pair<double, double>& p)
	: Angle(p.first, p.second) {
	double	r = hypot(p.first, p.second);
	_cos = p.first / r;
	_sin = p.second / r;
}

SinCos	SinCos::operator-() const {
	return SinCos(_cos, -_sin);
}

SinCos	SinCos::operator*(int k) const {
	if (k == 0) {
		return SinCos();
	}
	if (k > 0) {
		SinCos	x = *this;
		if (k == 1) {
			return x;
		}
		return x + x * (k-1);
	}
	SinCos	x = -*this;
	if (k == -1) {
		return x;
	}
	return x * (-k);
}

SinCos	SinCos::operator+(const SinCos& other) const {
	return SinCos(
		_cos * other._cos - _sin * other._sin,
		_sin * other._cos + _cos * other._sin
	);
}

SinCos	SinCos::operator-(const SinCos& other) const {
	return (*this) + (-other);
}

} // namespace solarsystem
} // namespace astro
