/*
 * ColorRectangles.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ColorRectangles.h"
#include <AstroDebug.h>

namespace snowgui {

//////////////////////////////////////////////////////////////////////
// Color implementation
//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////
// ColorChange implementation
//////////////////////////////////////////////////////////////////////
ColorChange::ColorChange(double y, double r, double g, double b)
	: Color(r, g, b), _y(y) {
}

ColorChange::ColorChange(double y, const Color& c) : Color(c), _y(y) {
}

ColorChange::ColorChange(double y, const QColor& c) : Color(c), _y(y) {
}

bool	ColorChange::operator<(const ColorChange& other) const {
	return _y < other._y;
}

//////////////////////////////////////////////////////////////////////
// ColorRectangles implementation
//////////////////////////////////////////////////////////////////////

void	ColorRectangles::draw(QPainter& painter, int width) const {
	Color	nextcolor;
	const_iterator	changeptr = begin();
	double	bottom = changeptr->y();
	nextcolor = nextcolor + *changeptr;
	while (end() != ++changeptr) {
		double	top = changeptr->y();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "draw from %f to %f",
			bottom, top);
		painter.fillRect(0, bottom, width, top - bottom,
			nextcolor.qcolor());
		nextcolor = nextcolor + *changeptr;
		bottom = top;
	} 
}

void	ColorRectangles::addRange(double bottom, double top, const Color& color){
	insert(ColorChange(bottom, color));
	insert(ColorChange(top, -color));
}

} // namespace snowgui
