/*
 * Point.cpp -- point implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <Format.h>
#include <debug.h>

namespace astro {

static const double	epsilon = 1e-10;

//////////////////////////////////////////////////////////////////////
// Point implementation
//////////////////////////////////////////////////////////////////////
Point	Point::operator+(const Point& other) const {
	return Point(x + other.x, y + other.y);
}

Point	Point::operator-(const Point& other) const {
	return Point(x - other.x, y - other.y);
}

Point	Point::operator*(double l) const {
	return Point(l * x, l * y);
}

Point	operator*(double l, const Point& other) {
	return other * l;
}

std::string	Point::toString() const {
	return stringprintf("(%f,%f)", x, y);
}

std::ostream&	operator<<(std::ostream& out, const Point& point) {
	return out << point.toString();
}

bool	Point::operator==(const Point& other) const {
	if (fabs(x - other.x) > epsilon) { return false; }
	if (fabs(y - other.y) > epsilon) { return false; }
	return true;
}

bool	Point::operator!=(const Point& other) const {
	return !operator==(other);
}

} // namespace astro
