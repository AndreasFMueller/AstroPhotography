/*
 * ImagePoint.cpp -- ImagePoint implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>

namespace astro {
namespace image {

/**
 * \brief compare points
 *
 * points are equal if both coordinates are equal
 */
bool	ImagePoint::operator==(const ImagePoint& other) const {
	return (x == other.x) && (y == other.y);
}

/**
 * \brief add points
 *
 * addition is just addition of components
 */
ImagePoint	ImagePoint::operator+(const ImagePoint& other) const {
	return ImagePoint(x + other.x, y + other.y);
}

/**
 * \brief subtract points
 *
 * addition is just subtraction of components
 */
ImagePoint	ImagePoint::operator-(const ImagePoint& other) const {
	return ImagePoint(x - other.x, y - other.y);
}

/**
 * \brief String representation
 */
std::string	ImagePoint::toString() const {
	return stringprintf("(%d,%d)", x, y);
}

/**
 * \brief Point output
 */
std::ostream&	operator<<(std::ostream& out, const ImagePoint& point) {
	return out << point.toString();
}

} // namespace image
} // namespace astro
