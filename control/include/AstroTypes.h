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
public:
	double	x;
	double	y;
	Point() : x(0), y(0) { }
	Point(double _x, double _y) : x(_x), y(_y) { }
	Point(const astro::image::ImagePoint& point) : x(point.x), y(point.y) {}
	Point	operator+(const Point& other) const;
	Point	operator-(const Point& other) const;
	Point	operator*(double l) const;
	friend Point	operator*(double l, const Point& other);
	std::string	toString() const;
	bool	operator==(const Point& other) const;
	bool	operator!=(const Point& other) const;
};
Point	operator*(double l, const Point& other);
std::ostream&	operator<<(std::ostream& out, const Point& other);

} // namespace astro

#endif /* _AstroTypes_h */
