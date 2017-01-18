/*
 * ColorArithmetic.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ColorArithmetic.h"
#include <AstroDebug.h>

namespace snowgui {

Color::Color() : _r(0), _g(0), _b(0) {
}

Color::Color(double r, double g, double b) : _r(r), _g(g), _b(b) {
}

Color::Color(const QColor& c) : _r(255. - c.red()),
	_g(255. - c.green()), _b(255. - c.blue()) {
}

Color	Color::operator+(const Color& other) const {
	return Color(_r + other._r, _g + other._g, _b + other._b);
}

Color	Color::operator*(const double f) const {
	return Color(_r * f, _g * f, _b * f);
}

QColor	Color::qcolor() const {
	return QColor(255. - _r, 255. - _g, 255. - _b);
}

Color	Color::operator-() const {
	return Color(-_r, -_g, -_b);
}

} // namespace snowgui
