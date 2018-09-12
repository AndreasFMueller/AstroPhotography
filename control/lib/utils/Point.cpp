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

Point	Point::operator*(const Point& other) const {
	return Point(_x * other._x, _y * other._y);
}

Point	operator*(double l, const Point& other) {
	return other * l;
}

std::string	Point::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

Point::operator	std::string() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const Point& point) {
	out << "(" << point.x() << "," << point.y() << ")";
	return out;
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

double	Point::abs() const {
	return hypot(_x, _y);
}

Point	Point::center(const std::set<Point>& points) {
	return 0.5 * (lowerleft(points) + upperright(points));
}

class LeftCompare {
public:
	bool	operator()(const Point& a, const Point& b) const {
		return (a.x() < b.x());
	}
};

class RightCompare {
public:
	bool	operator()(const Point& a, const Point& b) const {
		return (a.x() > b.x());
	}
};

class BottomCompare {
public:
	bool	operator()(const Point& a, const Point& b) const {
		return (a.y() < b.y());
	}
};

class TopCompare {
public:
	bool	operator()(const Point& a, const Point& b) const {
		return (a.y() > b.y());
	}
};

Point	Point::lowerleft(const std::set<Point>& points) {
	double	x = std::min_element(points.begin(), points.end(),
			LeftCompare())->x();
	double	y = std::min_element(points.begin(), points.end(),
			BottomCompare())->y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "left = %f, lower = %f", x, y);
	return Point(x, y);
}

Point	Point::lowerright(const std::set<Point>& points) {
	double	x = std::min_element(points.begin(), points.end(),
			RightCompare())->x();
	double	y = std::min_element(points.begin(), points.end(),
			BottomCompare())->y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "right = %f, lower = %f", x, y);
	return Point(x, y);
}

Point	Point::upperleft(const std::set<Point>& points) {
	double	x = std::min_element(points.begin(), points.end(),
			LeftCompare())->x();
	double	y = std::min_element(points.begin(), points.end(),
			TopCompare())->y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "left = %f, upper = %f", x, y);
	return Point(x, y);
}

Point	Point::upperright(const std::set<Point>& points) {
	double	x = std::min_element(points.begin(), points.end(),
			RightCompare())->x();
	double	y = std::min_element(points.begin(), points.end(),
			TopCompare())->y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "right = %f, upper = %f", x, y);
	return Point(x, y);
}

class PointAverager {
	Point	sum;
	unsigned int	counter;
public:
	PointAverager() { sum = Point(0, 0); counter = 0; }
	void	operator()(const Point& p) { sum = sum + p; counter++; }
	Point	average() const { return sum * (1. / counter); }
};

Point	Point::centroid(const std::set<Point>& points) {
	return for_each(points.begin(), points.end(), PointAverager()).average();
}

double	distance(const Point& p1, const Point& p2) {
	return hypot(p1.x() - p2.x(), p1.y() - p2.y());
}

double	azimut(const Point& from, const Point& to) {
	return atan2(to.y() - from.y(), to.x() - from.x());
}

} // namespace astro
