/*
 * ImagePoint.cpp -- ImagePoint implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

/**
 * \brief compare points
 *
 * points are equal if both coordinates are equal
 */
bool	ImagePoint::operator==(const ImagePoint& other) const {
	return (_x == other._x) && (_y == other._y);
}

/**
 * \brief add points
 *
 * addition is just addition of components
 */
ImagePoint	ImagePoint::operator+(const ImagePoint& other) const {
	return ImagePoint(_x + other._x, _y + other._y);
}

/**
 * \brief subtract points
 *
 * addition is just subtraction of components
 */
ImagePoint	ImagePoint::operator-(const ImagePoint& other) const {
	return ImagePoint(_x - other._x, _y - other._y);
}

/**
 * \brief String representation
 */
std::string	ImagePoint::toString() const {
	return stringprintf("(%u,%u)", _x, _y);
}

/**
 * \brief Point output
 */
std::ostream&	operator<<(std::ostream& out, const ImagePoint& point) {
	return out << point.toString();
}

std::istream&	operator>>(std::istream& in, ImagePoint& point) {
	unsigned int	x, y;
	char	c;
	in >> c >> x >> c >> y;
	point.setX(x);
	point.setY(y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsed image point: %s",
		point.toString().c_str());
	return in;
}

/**
 * \brief Ordering of points, needed so that points can be stored in sets
 */
bool	ImagePoint::operator<(const ImagePoint& other) const {
	if (_x < other._x) {
		return true;
	}
	if (_x > other._x) {
		return false;
	}
	return _y < other._y;
}

} // namespace image
} // namespace astro
