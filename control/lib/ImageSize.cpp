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
ImageSize::ImageSize(unsigned int width, unsigned int height)
	: _width(width), _height(height), pixels(width * height) {
}

/**
 * \brief Width accessor
 */
void	ImageSize::setWidth(unsigned int width) {
	_width = width;
	pixels = _width * _height;
}

/**
 * \brief Height accessor
 */
void	ImageSize::setHeight(unsigned int height) {
	_height = height;
	pixels = _width * _height;
}

/**
 * \brief Compare two size objects: equality
 *
 * Two size objects are equal if width and height are identical
 */
bool	ImageSize::operator==(const ImageSize& other) const {
	return (_width == other.width()) && (_height == other.height());
}

/**
 * \brief Compare two size objects: inequality
 *
 * Two size objects are unequal if width or height are unequal
 */
bool	ImageSize::operator!=(const ImageSize& other) const {
	return (_width != other.width()) || (_height != other.height());
}

/**
 * \brief Find out whether a point is contained in the rectangle
 *        defined by a size object
 */
bool	ImageSize::bounds(const ImagePoint& p) const {
	return	(0 <= p.x()) && (p.x() < _width) &&
		(0 <= p.y()) && (p.y() < _height);
}

/**
 * \brief Find out whether a rectangle is contained in the rectangle
 *        defined by a size ojbect.
 */
bool	ImageSize::bounds(const ImageRectangle& rect) const {
	if (!bounds(rect.origin())) {
		return false;
	}
	return bounds(ImagePoint(rect.origin().x() + rect.size().width(),
		rect.origin().y() + rect.size().height()));
}

/**
 * \brief Test whether a point is in the rectangle.
 *
 * \param point
 */
bool	ImageSize::contains(const ImagePoint& point) const {
	return contains(point.x(), point.y());
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
	if (x >= (int)_width) {
		return false;
	}
	if (y >= (int)_height) {
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
	return x + _width * y;
}

/**
 * \brief Find the offset into an array with this size
 */
unsigned int	ImageSize::offset(const ImagePoint& point) const {
	return offset(point.x(), point.y());
}

/**
 * \brief String representation
 */
std::string	ImageSize::toString() const {
	return stringprintf("%dx%d", _width, _height);
}

} // namespace image
} // namespace astro
