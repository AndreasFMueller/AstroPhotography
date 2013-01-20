/*
 * ImageSize.cpp -- ImageSize implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Construct a size oject based on width and height
 *
 * An ImageSize object also keeps track of the number of
 * pixels contained in it. Since this number is used very
 * often, keeping a it redundantly in memory saves a large
 * number of multiplications
 */
ImageSize::ImageSize(int _width, int _height)
	: width(_width), height(_height), pixels(width * height) {
}

/**
 * \brief Compare two size objects: equality
 *
 * Two size objects are equal if width and height are identical
 */
bool	ImageSize::operator==(const ImageSize& other) const {
	return (width == other.width) && (height == other.height);
}

/**
 * \brief Compare two size objects: inequality
 *
 * Two size objects are unequal if width or height are unequal
 */
bool	ImageSize::operator!=(const ImageSize& other) const {
	return (width != other.width) || (height != other.height);
}

/**
 * \brief Find out whether a point is contained in the rectangle
 *        defined by a size object
 */
bool	ImageSize::bounds(const ImagePoint& p) const {
	return	(0 <= p.x) && (p.x < width) &&
		(0 <= p.y) && (p.y < height);
}

/**
 * \brief Find out whether a rectangle is contained in the rectangle
 *        defined by a size ojbect.
 */
bool	ImageSize::bounds(const ImageRectangle& rect) const {
	if (!bounds(rect.origin)) {
		return false;
	}
	return bounds(ImagePoint(rect.origin.x + rect.size.width,
		rect.origin.y + rect.size.height));
}

} // namespace image
} // namespace astro
