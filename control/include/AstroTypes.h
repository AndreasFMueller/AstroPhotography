/*
 * AstroTypes.h -- some commonly used types, but not any types special to images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTypes_h
#define _AstroTypes_h

#include <AstroImage.h>
#include <iostream>
#include <set>

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
	Point(double angle) : _x(cos(angle)), _y(sin(angle)) { }
	double	x() const { return _x; }
	double	y() const { return _y; }
	double	abs() const;
	void	setX(double x) { _x = x; }
	void	setY(double y) { _y = y; }
	Point	operator+(const Point& other) const;
	Point	operator-(const Point& other) const;
	Point	operator-() const;
	Point	operator*(double l) const;
	Point	operator*(const Point& other) const;
	friend Point	operator*(double l, const Point& other);
	std::string	toString() const;
	operator	std::string() const;
	bool	operator==(const Point& other) const;
	bool	operator!=(const Point& other) const;
	operator double() const;
	static Point	center(const std::set<Point>& points);
	static Point	lowerleft(const std::set<Point>& points);
	static Point	lowerright(const std::set<Point>& points);
	static Point	upperleft(const std::set<Point>& points);
	static Point	upperright(const std::set<Point>& points);
	static Point	centroid(const std::set<Point>& points);
	void	normalize();
	Point	normalized() const;
};
Point	operator*(double l, const Point& other);
double	distance(const Point& p1, const Point& p2);
double	azimut(const Point& from, const Point& to);

std::ostream&	operator<<(std::ostream& out, const Point& other);
std::istream&	operator>>(std::istream& in, Point& other);

/**
 * \brief Dimensions of a rectangle in double coordinates
 */
class Size {
	double	_width;
	double	_height;
	void	setup(const Point& lowerleft, const Point& upperright);
public:
	Size(double width, double height) : _width(width), _height(height) { }
	Size(const Point& lowerleft, const Point& upperright);
	Size(const std::set<Point>& points);
	double width() const { return _width; }
	void	width(double w) { _width = w; }
	double height() const { return _height; }
	void	height(double h) { _height = h; }
	bool	contains(const Point& point) const;
	std::string	toString() const;
};
std::ostream&	operator<<(std::ostream& out, const Size& size);

/**
 * \brief A rectangle in arbitrary coordinates
 */
class Rectangle: public Point, public Size {
public:
	Rectangle(Point origin, Size size) : Point(origin), Size(size) { }
	Rectangle(const std::set<Point>& points);
	Point	origin() const { return Point(*this); }
	Size	size() const { return Size(*this); }
	bool	contains(const Point& point) const;
	std::string	toString() const;
};
std::ostream&	operator<<(std::ostream& out, const Rectangle& rectangle);

/**
 * \brief Rotation operation on points
 */
class Rotation {
	double	_alpha;
public:
	Rotation(double alpha) : _alpha(alpha) { }
	Point	operator()(const Point& p) const;
};

/**
 * \brief Temperature conversion class
 */
class Temperature {
	float	_temperature;
public:
	typedef enum { KELVIN, CELSIUS } temperature_scale;
	const static float	zero;
	Temperature(float temperature, temperature_scale scale = KELVIN);
	float	celsius() const;
	float	temperature() const { return _temperature; }
};

} // namespace astro

#endif /* _AstroTypes_h */
