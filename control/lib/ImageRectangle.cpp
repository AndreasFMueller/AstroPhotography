/*
 * ImageRectangle.cpp -- ImageRectangle implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <sstream>
#include <regex.h>

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
 * \brief Parse a rectangle specification
 *
 * Rectangle specification mimic the way X11 specifies the geometry of a window.
 * A correct rectangle specification is of the form widthxheight@(x,y).
 */
ImageRectangle::ImageRectangle(const std::string& rectanglespec) {
	int	rc = 0;
	const char	*r = "([0-9]+)x([0-9]+)@\\(?([0-9]+),([0-9]+)\\)?";
	regex_t	regex;
	if (regcomp(&regex, r, REG_EXTENDED)) {
		throw std::runtime_error("internal error: RE does not compile");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "matching '%s' against re '%s'",
		rectanglespec.c_str(), r);
#define nmatches	5
	regmatch_t	matches[nmatches];
	rc = regexec(&regex, rectanglespec.c_str(), nmatches, matches, 0);
#if 0
	for (int i = 0; i < 5; i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d]: %d - %d", i,
			matches[i].rm_so, matches[i].rm_eo);
	}
#endif
	int	width, height, x, y;
	if (rc) {
		goto cleanup;
	}
	width = std::stoi(rectanglespec.substr(matches[1].rm_so,
			matches[1].rm_eo - matches[1].rm_so));
	height = std::stoi(rectanglespec.substr(matches[2].rm_so,
			matches[2].rm_eo - matches[2].rm_so));
	_size = ImageSize(width, height);
	x = std::stoi(rectanglespec.substr(matches[3].rm_so,
			matches[3].rm_eo - matches[3].rm_so));
	y = std::stoi(rectanglespec.substr(matches[4].rm_so,
			matches[4].rm_eo - matches[4].rm_so));
	_origin = ImagePoint(x, y);
cleanup:
	regfree(&regex);
	if (rc) {
		throw std::runtime_error("ImageRectangle: no match");
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
	std::ostringstream	out;
	out << *this;
	return out.str();
	//return stringprintf("%s@%s", _size.toString().c_str(),
	//	_origin.toString().c_str());
}

ImageRectangle::operator	std::string() const {
	return toString();
}

std::ostream&	operator<<(std::ostream& out, const ImageRectangle& rectangle) {
	out << rectangle.size() << "@" << rectangle.origin();
	return out;
}

std::istream&	operator>>(std::istream& in, ImageRectangle& rectangle) {
	ImageSize	size;
	in >> size;
	// the next component must be a single '@' character
	char	c;
	in >> c;
	if (c != '@') {
		throw std::runtime_error("not a rectangle specification");
	}
	ImagePoint	origin;
	in >> origin;

	// set the result
	rectangle.setSize(size);
	rectangle.setOrigin(origin);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsed rectangle spec %s",
		rectangle.toString().c_str());
	return in;
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

ImagePoint	ImageRectangle::center() const {
	return _origin + _size.center();
}

ImagePoint	ImageRectangle::subimage(unsigned int x, unsigned int y) const {
	if (!_size.contains(x, y)) {
		throw std::runtime_error("outside image");
	}
	return ImagePoint(_origin.x() + x, _origin.y() + y);
}

ImagePoint	ImageRectangle::subimage(const ImagePoint& point) const {
	return subimage(point.x(), point.y());
}

} // namespace image
} // namespace astro
