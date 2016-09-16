/*
 * Triangle.cpp -- implementation of the triangle data structure for
 *                 the triangle matching algorithm
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <sstream>

namespace astro {
namespace image {
namespace transform {

class NoCommonPoint : std::exception {
public:
	NoCommonPoint() { }
};

/**
 * \brief Auxiliary class to find the order of points in a triangle
 */
class	PointPairDistance {
	Point	p1;
	Point	p2;
	double	d;
public:
	PointPairDistance(const Point& a, const Point& b) : p1(a), p2(b) {
		d = distance(a, b);
	}
	bool	operator<(const PointPairDistance& other) const {
		return d > other.d;
	}
	Point	common(const PointPairDistance& other) const {
		if ((p1 == other.p1) || (p1 == other.p2)) {
			return p1;
		}
		if ((p2 == other.p1) || (p2 == other.p2)) {
			return p2;
		}
		throw NoCommonPoint();
	}
};

/**
 * \brief Constructor of a triangle
 *
 * The triangle first orders the points in such a way that
 * 1. the first point is the common end point of the two longest sides
 * 2. the third point is the common endpoint of the two shortest sides
 * it the computes the triangle paramters
 */
Triangle::Triangle(Point p1, Point p2, Point p3) {
	std::set<PointPairDistance>	pointpairs;
	pointpairs.insert(pointpairs.begin(), PointPairDistance(p1, p2));
	pointpairs.insert(pointpairs.begin(), PointPairDistance(p1, p3));
	pointpairs.insert(pointpairs.begin(), PointPairDistance(p2, p3));
	try {
		auto ptr = pointpairs.begin();
		PointPairDistance	d1 = *ptr++;
		PointPairDistance	d2 = *ptr++;
		PointPairDistance	d3 = *ptr;
		points.push_back(d1.common(d2));
		points.push_back(d1.common(d3));
		points.push_back(d2.common(d3));
	} catch (const NoCommonPoint& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "logic error, no common point");
		throw;
	}
	_azimut = astro::azimut(points[0], points[1]);
	_angle = astro::azimut(points[0], points[2]) - _azimut;
	if (_angle > 0.25 * M_PI) {
		_angle = _angle - 2 * M_PI;
	}
	_longside = astro::distance(points[0], points[1]);
	_middleside = astro::distance(points[0], points[2]) / _longside;
	_area = 0.5 * _longside * _middleside * fabs(sin(_angle));
}

bool	Triangle::mirror_to(const Triangle& other) const {
	return (_angle * other._angle) < 0;
}

double	Triangle::rotate_to(const Triangle& other) const {
	double	rot = other.azimut() - azimut();
	while (rot < 0) {
		rot += 2 * M_PI;
	}
	while (rot > 2 * M_PI) {
		rot -= 2 * M_PI;
	}
	return rot;
}

double	Triangle::scale_to(const Triangle& other) const {
	return other.longside() / longside();
}

/**
 * \brief Transform from current triangle to the target triangle
 */
Transform	Triangle::to(const Triangle& target) const {
	double	rotation = rotate_to(target);
	double	scale = scale_to(target);
	Transform	result(rotation, Point(), scale);
	Point	translation = target.basepoint() - result(basepoint());
	result = result + translation;
	return result;
}

/**
 * \brief Order operator for Triangle
 *
 * This is lexicographic order on angle() and middleside()
 */
bool	Triangle::operator<(const Triangle& other) const {
	if (_angle < other._angle) {
		return true;
	}
	if (_angle > other._angle) {
		return false;
	}
	return _middleside < other._middleside;
}

/**
 * \brief Distance between triangles
 *
 * This is essentially euclidean distance in the (_angle, _middleside)-plane.
 */
double	Triangle::distance(const Triangle& other) const {
	return hypot(other._angle - _angle,
		other._middleside - _middleside);
}

std::string	Triangle::toString() const {
	std::ostringstream	out;
	out << points[0];
	out << ", ";
	out << points[1];
	out << ", ";
	out << points[2];
	out << ", ";
	out << " longside=" << _longside;
	out << " middleside=" << _middleside;
	out << " angle=" << _angle * 180 / M_PI;
	return out.str();
}

Triangle::operator	std::string() const {
	return toString();
}

} // namespace transform
} // namespace image
} // namespace astro
