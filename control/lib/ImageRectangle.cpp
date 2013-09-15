/*
 * ImageRectangle.cpp -- ImageRectangle implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>

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
	: _origin(rectangle.origin() + translatedby),
	  _size(rectangle.size()) { }

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
	: _origin(rectangle.origin() + subrectangle.origin()),
	  _size(subrectangle.size()) {
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
	return (_origin == other.origin()) && (_size == other.size());
}

/**
 * \brief Test whether a point is contained within a rectangle
 */
bool	ImageRectangle::contains(const ImagePoint& point) const {
	return	(_origin.x() <= point.x()) &&
		(point.x() < _origin.x() + _size.width()) &&
		(_origin.y() <= point.y()) &&
		(point.y() < _origin.y() + _size.height());
}

/**
 * \brief Test whether a rectangle is contained within a rectangle
 */
bool	ImageRectangle::contains(const ImageRectangle& other) const {
	return	(_origin.x() <= other.origin().x()) &&
		(_origin.x() + _size.width() >= other.origin().x() + other.size().width()) &&
		(_origin.y() <= other.origin().y()) &&
		(_origin.y() + _size.height() >= other.origin().y() + other.size().height());
}

/**
 * \brief Extract the lower left corner (the origin) of a rectangle
 */
const ImagePoint&	ImageRectangle::lowerLeftCorner() const {
	return _origin;
}

/**
 * \brief Extract the lower right corner of a rectangle
 */
ImagePoint	ImageRectangle::lowerRightCorner() const {
	return ImagePoint(_origin.x() + _size.width() - 1, _origin.y());
}

/**
 * \brief Extract the upper left corner of a rectangle
 */
ImagePoint	ImageRectangle::upperLeftCorner() const {
	return ImagePoint(_origin.x(), _origin.y() + _size.height() - 1);
}

/**
 * \brief Extract the upper right corner of a rectangle
 */
ImagePoint	ImageRectangle::upperRightCorner() const {
	return ImagePoint(_origin.x() + _size.width() - 1,
			_origin.y() + _size.height() - 1);
}

/**
 * \brief string representation of the rectangle
 */
std::string	ImageRectangle::toString() const {
	return stringprintf("%s@%s", _size.toString().c_str(),
		_origin.toString().c_str());
}

ImagePoint      ImageRectangle::upperright() const {
	return _origin + _size.upperright();
}

ImagePoint      ImageRectangle::upperleft() const {
	return _origin + _size.upperleft();
}

ImagePoint      ImageRectangle::lowerleft() const {
	return _origin + _size.lowerleft();
}

ImagePoint      ImageRectangle::lowerright() const {
	return _origin + _size.lowerright();
}

} // namespace image
} // namespace astro
