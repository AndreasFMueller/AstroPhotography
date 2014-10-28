/*
 * ImagePoint.cpp -- ImagePoint implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include <astroregex.h>

namespace astro {
namespace image {

/**
 * \brief Construct an ImagePoint from a point specification
 *
 * A valid specification is (x,y), where the parentheses are
 * option.
 */
ImagePoint::ImagePoint(const std::string& pointspec) {
	std::string	r("\\(?([0-9]+),([0-9]+)\\)?");
	astro::regex	regex(r, astro::regex::extended);
	astro::smatch	matches;
	if (!regex_match(pointspec, matches, regex)) {
		std::string	msg = stringprintf("bad pointspec '%s'",
			pointspec.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	_x = std::stoi(matches[1]);
	_y = std::stoi(matches[2]);
}

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
	char	p1, c, p2;
	in >> p1 >> x >> c >> y >> p2;
	if ((p1 != '(') || (c != ',') || (p2 != ')')) {
		throw std::runtime_error("not a point specification");
	}
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

/**
 * \brief Compute distances
 */
float	ImagePoint::distance(const ImagePoint& p) const {
	return hypot((_x < p.x()) ? (p.x() - _x) : (_x - p.x()),
			(_y < p.y()) ? (p.y() - _y) : (_y - p.y()));
}

} // namespace image
} // namespace astro
