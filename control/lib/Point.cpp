/*
 * Point.cpp -- point implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <sstream>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {

static const double	epsilon = 1e-10;

//////////////////////////////////////////////////////////////////////
// Point implementation
//////////////////////////////////////////////////////////////////////
Point	Point::operator+(const Point& other) const {
	return Point(_x + other._x, _y + other._y);
}

Point	Point::operator-(const Point& other) const {
	return Point(_x - other._x, _y - other._y);
}

Point	Point::operator-() const {
	return Point(-_x, -_y);
}

Point	Point::operator*(double l) const {
	return Point(l * _x, l * _y);
}

Point	operator*(double l, const Point& other) {
	return other * l;
}

std::string	Point::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const Point& point) {
	out << "(" << point.x() << "," << point.y() << ")";
}

bool	Point::operator==(const Point& other) const {
	if (fabs(_x - other._x) > epsilon) { return false; }
	if (fabs(_y - other._y) > epsilon) { return false; }
	return true;
}

bool	Point::operator!=(const Point& other) const {
	return !operator==(other);
}

Point::operator double() const {
	return hypot(_x, _y);
}

std::istream&	operator>>(std::istream& in, Point& point) {
	double	x, y;
	absorb(in, '(');
	in >> x;
	absorb(in, ',');
	in >> y;
	absorb(in, ')');
	point.setX(x);
	point.setY(y);
	return in;
}

} // namespace astro
