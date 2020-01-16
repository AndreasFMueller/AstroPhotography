/*
 * ImageRectangle.cpp -- ImageRectangle implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <sstream>
#include <regex>

namespace astro {
namespace image {

/**
 * \brief Construct an ImageRectangle with a given boundary distance
 *
 * \param size			the size of the original image
 * \param boundarydistance	the distance to the boundary
 */
ImageRectangle::ImageRectangle(const ImageSize& size, int boundarydistance)
	: _origin(boundarydistance, boundarydistance),
	  _size(size.width() - 2 * boundarydistance,
		size.height() - 2 * boundarydistance) {
}

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
	std::string	r("([0-9]+)x([0-9]+)@\\(?([0-9]+),([0-9]+)\\)?");
	std::regex	regex(r, std::regex::extended);
	std::smatch	matches;
	if (!regex_match(rectanglespec, matches, regex)) {
		std::string	msg = stringprintf("bad rectangle spec '%s'", 
			rectanglespec.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	int	width = std::stoi(matches[1]);
	int	height = std::stoi(matches[2]);
	_size = ImageSize(width, height);
	int	x = std::stoi(matches[3]);
	int	y = std::stoi(matches[4]);
	_origin = ImagePoint(x, y);
}

/**
 * \brief Assignment operator for ImageRectangles
 *
 * \param other		the other rectangle to asign to this one
 */
ImageRectangle&	ImageRectangle::operator=(const ImageRectangle& other) {
	_origin = other.origin();
	_size = other.size();
	return *this;
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

bool	ImageRectangle::operator!=(const ImageRectangle& other) const {
	if (_origin == other._origin) {
		return false;
	}
	if (_size == other._size) {
		return false;
	}
	return true;
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
 * \brief check whether coordinates are inside the rectangle
 */
bool	ImageRectangle::contains(int x, int y) const {
	return	(_origin.x() <= x) &&
		(x < _origin.x() + _size.width()) &&
		(_origin.y() <= y) &&
		(y < _origin.y() + _size.height());
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

bool	ImageRectangle::fits(const ImageSize& size) const {
	if ((_origin.x() < 0) || (_origin.y() < 0)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"lower left corner %s outside image of size %s",
			_origin.toString().c_str(), size.toString().c_str());
		return false;
	}
	ImagePoint	ur = upperRightCorner();
	if (!size.contains(ur)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"upper right corner %s outside image of size %s",
			ur.toString().c_str(), size.toString().c_str());
		return false;
	}
	return true;
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

ImagePoint	ImageRectangle::subimage(int x, int y) const {
	if (!_size.contains(x, y)) {
		throw std::runtime_error("outside image");
	}
	return ImagePoint(_origin.x() + x, _origin.y() + y);
}

ImagePoint	ImageRectangle::subimage(const ImagePoint& point) const {
	return subimage(point.x(), point.y());
}

int	ImageRectangle::borderDistance(const ImagePoint& point) const {
	ImagePoint	upper = upperright();
	int	m, n;
	n = point.x() - _origin.x();		m = n;
	n = point.y() - _origin.y();		if (n < m) { m = n; }
	n = upper.x() - point.x();		if (n < m) { m = n; }
	n = upper.y() - point.y();		if (n < m) { m = n; }
	return m;
}

ImagePoint	ImageRectangle::centerWithinFrame(const ImageSize& frame) const {
	ImagePoint	p = frame.center() - origin();
	return p;
}

int	ImageRectangle::xmin() const {
	return _origin.x();
}

int	ImageRectangle::ymin() const {
	return _origin.y();
}

int	ImageRectangle::xmax() const {
	return _origin.x() + _size.width();
}

int	ImageRectangle::ymax() const {
	return _origin.y() + _size.height();
}

} // namespace image
} // namespace astro
