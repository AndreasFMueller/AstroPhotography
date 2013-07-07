/*
 * ImageRectangle.cpp -- ImageRectangle implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $id$
 */
#include <AstroImage.h>
#include <Format.h>

namespace astro {
namespace image {

/**
 * \brief Construct a translated rectangle
 *
 * The new rectangle will be a rectangle of the same dimensions with
 * the translated by the second argument.
 * \param rectangle	source rectangle
 * \param translatedby	the vector by which to translate the rectangle
 */
ImageRectangle::ImageRectangle(const ImageRectangle& rectangle,
	const ImagePoint& translatedby)
	: origin(rectangle.origin + translatedby),
	  size(rectangle.size) { }

/**
 * \brief Construct a subrectangle relative to the source rectangles 
 *        coordinate system
 *
 * \param rectangle	source rectangle
 * \param subrectangle	rectangle within a coordinate system of the
 *                      source rectangle (lower left corner at (0,0))
 */
ImageRectangle::ImageRectangle(const ImageRectangle& rectangle,
	const ImageRectangle& subrectangle)
	: origin(rectangle.origin + subrectangle.origin),
	  size(subrectangle.size) {
	if (!rectangle.contains(subrectangle)) {
		throw std::range_error("subrectangle not contained in rectangle");
	}
}

/**
 * \brief Rectangle comparison
 *
 * rectangles are considered equal if the have the same origin and the same
 * size.
 */
bool	ImageRectangle::operator==(const ImageRectangle& other) const {
	return (origin == other.origin) && (size == other.size);
}

/**
 * \brief Test whether a point is contained within a rectangle
 */
bool	ImageRectangle::contains(const ImagePoint& point) const {
	return	(origin.x <= point.x) && (point.x < origin.x + size.width) &&
		(origin.y <= point.y) && (point.y < origin.y + size.height);
}

/**
 * \brief Test whether a rectangle is contained within a rectangle
 */
bool	ImageRectangle::contains(const ImageRectangle& other) const {
	return	(origin.x <= other.origin.x) &&
		(origin.x + size.width >= other.origin.x + other.size.width) &&
		(origin.y <= other.origin.y) &&
		(origin.y + size.height >= other.origin.y + other.size.height);
}

/**
 * \brief Extract the lower left corner (the origin) of a rectangle
 */
const ImagePoint&	ImageRectangle::lowerLeftCorner() const {
	return origin;
}

/**
 * \brief Extract the lower right corner of a rectangle
 */
ImagePoint	ImageRectangle::lowerRightCorner() const {
	return ImagePoint(origin.x + size.width - 1, origin.y);
}

/**
 * \brief Extract the upper left corner of a rectangle
 */
ImagePoint	ImageRectangle::upperLeftCorner() const {
	return ImagePoint(origin.x, origin.y + size.height - 1);
}

/**
 * \brief Extract the upper right corner of a rectangle
 */
ImagePoint	ImageRectangle::upperRightCorner() const {
	return ImagePoint(origin.x + size.width - 1, origin.y + size.height - 1);
}

/**
 * \brief string representation of the rectangle
 */
std::string	ImageRectangle::toString() const {
	return stringprintf("%s@%s", size.toString().c_str(),
		origin.toString().c_str());
}

} // namespace image
} // namespace astro
