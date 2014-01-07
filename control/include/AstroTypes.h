/*
 * AstroTypes.h -- some commonly used types, but not any types special to images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTypes_h
#define _AstroTypes_h

#include <AstroImage.h>
#include <iostream>

using namespace astro::image;

/**
 * \brief The astro namespace is used for all classes of the astro library
 *
 * A client of the CORBA server does not use these classes, but uses the
 * interfaces defined in the IDL instead. Those interfaces are all in the
 * the namespace Astro (capital A).
 *
 * Some of the definitions in the Astro namespace have corresponding
 * definitions in the astro namespace, although with slight modifications
 * e.g. to handle access restrictions or type restrictions. There is 
 * a set of conversion functions to bridge the gap between the two
 * namespaces.
 */
namespace astro {

/**
 * \brief Point with noninteger coordinates
 *
 * Such points are needed when registering images.
 */

class Point {
	double	_x;
	double	_y;
public:
	Point() : _x(0), _y(0) { }
	Point(double x, double y) : _x(x), _y(y) { }
	Point(const astro::image::ImagePoint& point)
		: _x(point.x()), _y(point.y()) {}
	double	x() const { return _x; }
	double	y() const { return _y; }
	void	setX(double x) { _x = x; }
	void	setY(double y) { _y = y; }
	Point	operator+(const Point& other) const;
	Point	operator-(const Point& other) const;
	Point	operator-() const;
	Point	operator*(double l) const;
	friend Point	operator*(double l, const Point& other);
	std::string	toString() const;
	bool	operator==(const Point& other) const;
	bool	operator!=(const Point& other) const;
	operator double() const;
};
Point	operator*(double l, const Point& other);

std::ostream&	operator<<(std::ostream& out, const Point& other);
std::istream&	operator>>(std::istream& in, Point& other);

} // namespace astro

#endif /* _AstroTypes_h */
