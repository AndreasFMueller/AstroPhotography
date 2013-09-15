/*
 * AstroTypes.h -- some commonly used types, but not any types special to images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroTypes_h
#define _AstroTypes_h

#include <AstroImage.h>

using namespace astro::image;

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

/**
 * \brief Name of a device
 */
class DeviceName {
private:
	std::string	_modulename;
public:
	const std::string&	modulename() const { return _modulename; }
	
private:
	std::string	_unitname;
public:
	const std::string&	unitname() const { return _unitname; }
public:
	DeviceName(const std::string& name);
	DeviceName(const std::string& modulename, const std::string& unitname)
		: _modulename(modulename), _unitname(unitname) { }
	// comparison operators (for containers)
	bool	operator==(const DeviceName& other) const;
	bool	operator!=(const DeviceName& other) const;
	bool	operator<(const DeviceName& other) const;
	// cast to a string
	operator std::string() const;
};

} // namespace astro

#endif /* _AstroTypes_h */
