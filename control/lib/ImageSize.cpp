/*
 * ImageSize.cpp -- ImageSize implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>

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
ImageSize::ImageSize(unsigned int _width, unsigned int _height)
	: width(_width), height(_height), pixels(width * height) {
}

/**
 * \brief Width accessor
 */
void	ImageSize::setWidth(unsigned int _width) {
	width = _width;
	pixels = width * height;
}

/**
 * \brief Height accessor
 */
void	ImageSize::setHeight(unsigned int _height) {
	height = _height;
	pixels = width * height;
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

/**
 * \brief Test whether a point is in the rectangle.
 *
 * \param point
 */
bool	ImageSize::contains(const ImagePoint& point) const {
	return contains(point.x, point.y);
}

/**
 * \brief Test whether a coordinate pair is in the rectangle.
 *
 * \param x
 * \param y
 */
bool	ImageSize::contains(int x, int y) const {
	if (x < 0) {
		return false;
	}
	if (y < 0) {
		return false;
	}
	if (x >= (int)width) {
		return false;
	}
	if (y >= (int)height) {
		return false;
	}
	return true;
}

/**
 * \brief Characteristic function for the image rectangle.
 *
 * This method is useful for debayering algorithms.
 * \param x
 * \param y
 */
int	ImageSize::chi(unsigned int x, unsigned int y) const {
	return contains(x, y) ? 1 : 0;
}

/**
 * \brief Find the offset into an array with this size
 */
unsigned int	ImageSize::offset(unsigned int x, unsigned int y) const {
	return x + width * y;
}

/**
 * \brief Find the offset into an array with this size
 */
unsigned int	ImageSize::offset(const ImagePoint& point) const {
	return offset(point.x, point.y);
}

/**
 * \brief String representation
 */
std::string	ImageSize::toString() const {
	return stringprintf("%dx%d", width, height);
}

} // namespace image
} // namespace astro
