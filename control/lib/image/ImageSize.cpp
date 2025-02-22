/*
 * ImageSize.cpp -- ImageSize implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace image {

ImageSize::ImageSize() : _width(0), _height(0) {
}

/**
 * \brief Construct a size oject based on width and height
 *
 * An ImageSize object also keeps track of the number of
 * pixels contained in it. Since this number is used very
 * often, keeping a it redundantly in memory saves a large
 * number of multiplications
 *
 * \param width		the width of the size rectangle
 * \param height	the height of the size rectangle
 */
ImageSize::ImageSize(unsigned int width, unsigned int height)
	: _width(width), _height(height), pixels(width * height) {
}

/**
 * \brief Construct the size based on a size specification
 *
 * Valid size specifications are of the form widthxheight
 */
ImageSize::ImageSize(const std::string& sizespec) {
	std::string::size_type	offset = sizespec.find('x');
	if (offset == std::string::npos) {
		throw std::runtime_error("not a size specification");
	}
	_width = std::stoi(sizespec.substr(0, offset));
	_height = std::stoi(sizespec.substr(offset + 1));
	if ((_width < 0) || (_height < 0)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "negative image dimensions");
		throw std::runtime_error("negative image dimensions");
	}
	pixels = _width * _height;
}

/**
 * \brief Construct a square size
 */
ImageSize::ImageSize(unsigned int width_and_height)
	: _width(width_and_height), _height(width_and_height),
	  pixels(width_and_height * width_and_height) {
}

ImageSize::ImageSize(const ImageSize& other)
	: _width(other._width), _height(other._height) {
	pixels = _width * _height;
}

ImageSize&	ImageSize::operator=(const ImageSize&other) {
	_width = other._width;
	_height = other._height;
	pixels = _width * _height;
	return *this;
}

/**
 * \brief Destructor
 */
ImageSize::~ImageSize() {
}

/**
 * \brief Width accessor
 */
void	ImageSize::setWidth(int width) {
	_width = width;
	pixels = _width * _height;
}

/**
 * \brief Height accessor
 */
void	ImageSize::setHeight(int height) {
	_height = height;
	pixels = _width * _height;
}

/**
 * \brief Compare two size objects: equality
 *
 * Two size objects are equal if width and height are identical
 *
 * \param other		the size to compare with
 */
bool	ImageSize::operator==(const ImageSize& other) const {
	return (_width == other.width()) && (_height == other.height());
}

/**
 * \brief Compare two size objects: inequality
 *
 * Two size objects are unequal if width or height are unequal
 *
 * \param other		the size to compare with
 */
bool	ImageSize::operator!=(const ImageSize& other) const {
	return (_width != other.width()) || (_height != other.height());
}

/**
 * \brief Size comparison 
 *
 * returns true if width and height are at least as large as those of other
 *
 * \param other		the size to compare with
 */
bool	ImageSize::operator>=(const ImageSize& other) const {
	return (_width >= other.width()) && (_height >= other.height());
}

/**
 * \brief Find out whether a point is contained in the rectangle
 *        defined by a size object
 */
bool	ImageSize::bounds(const ImagePoint& p) const {
	return	(0 <= p.x()) && (p.x() < _width) && (0 <= p.y()) && (p.y() < _height);
}

/**
 * \brief Find out whether a rectangle is contained in the rectangle
 *        defined by a size ojbect.
 */
bool	ImageSize::bounds(const ImageRectangle& rect) const {
	if (!bounds(rect.origin())) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "origin outside");
		return false;
	}
	return bounds(ImagePoint(rect.origin().x() + rect.size().width() - 1,
		rect.origin().y() + rect.size().height() - 1));
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

ImagePoint      ImageSize::upperright() const {
	return ImagePoint(_width - 1, _height - 1);
}

ImagePoint      ImageSize::upperleft() const {
	return ImagePoint(0, _height - 1);
}

ImagePoint      ImageSize::lowerleft() const {
	return ImagePoint(0, 0);
}

ImagePoint      ImageSize::lowerright() const {
	return ImagePoint(_width - 1, 0);
}

ImagePoint	ImageSize::center() const {
	return ImagePoint(_width / 2, _height / 2);
}

ImageSize	ImageSize::operator*(const double l) const {
	unsigned int	x = _width * l;
	unsigned int	y = _height * l;
	return ImageSize(x, y);
}

ImageSize	ImageSize::operator/(const double l) const {
	unsigned int	x = _width / l;
	unsigned int	y = _height / l;
	return ImageSize(x, y);
}

std::ostream&	operator<<(std::ostream& out, const ImageSize& size) {
	out << size.width() << "x" << size.height();
	return out;
}

std::istream&	operator>>(std::istream& in, ImageSize& size) {
	unsigned int	width, height;
	char	x;
	in >> width >> x >> height;
	if (x != 'x') {
		throw std::runtime_error("not a size specification");
	}
	size.setWidth(width);
	size.setHeight(height);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parsed image size: %s",
		size.toString().c_str());
	return in;
}

/**
 * \brief Reduce a point to point insed the image
 */
ImagePoint	ImageSize::operator()(const int x, const int y) const {
	int	xr = x;
	while (xr < 0) { xr += _width; }
	xr = xr % _width;
	int	yr = y;
	while (yr < 0) { yr += _height; }
	yr = yr % _height;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "reduce (%d,%d) to (%d,%d)",
	//	x, y, xr, yr);
	return ImagePoint(xr, yr);
}
ImagePoint	ImageSize::operator()(const ImagePoint& p) const {
	ImagePoint	result = (*this)(p.x(), p.y());
	return result;
}

ImagePoint	ImageSize::flip(const ImagePoint& p) const {
	return ImagePoint(_width - 1 - p.x(), _height - 1 - p.y());
}

ImagePoint	ImageSize::horizontalFlip(const ImagePoint& p) const {
	return ImagePoint(_width - 1 - p.x(), p.y());
}

ImagePoint	ImageSize::verticalFlip(const ImagePoint& p) const {
	return ImagePoint(p.x(), _height - 1 - p.y());
}

/**
 * \brief Find the number of pixels to the border
 *
 * This function returns negative values if the point is outside the
 * range defined by the size.
 */
int	ImageSize::borderDistance(const ImagePoint& point) const {
	int	m, n;
	n = point.x(); 			m = n;
	n = _width - 1 - point.x();	if (n < m) { m = n; }
	n = point.y();			if (n < m) { m = n; }
	n = _height - 1 - point.y();	if (n < m) { m = n; }
	return m;
}

/**
 * \brief Find the intersection of the size with the rectangle
 *
 * \param rectangle	make this rectangle fit in the size
 */
ImageRectangle	ImageSize::containing(const ImageRectangle& rectangle) const {
	int	w = rectangle.size().width();
	int	h = rectangle.size().height();
	int	x = rectangle.size().width() + rectangle.origin().x();
	int	y = rectangle.size().height() + rectangle.origin().y();
	if (x > width()) {
		w = width() - rectangle.origin().x();
	}
	if (y > height()) {
		h = height() - rectangle.origin().y();
	}
	return ImageRectangle(rectangle.origin(), ImageSize(w, h));
}

} // namespace image
} // namespace astro
