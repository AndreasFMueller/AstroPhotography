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
#include <regex.h>

namespace astro {
namespace image {

/**
 * \brief Construct an ImagePoint from a point specification
 *
 * A valid specification is (x,y), where the parentheses are
 * option.
 */
ImagePoint::ImagePoint(const std::string& pointspec) {
	int	rc = 0;
	const char	*r = "\\(?([0-9]+),([0-9]+)\\)?";
	regex_t	regex;
	if (regcomp(&regex, r, REG_EXTENDED)) {
		throw std::runtime_error("internal error, RE does not compile");
	}
#define nmatches 3
	regmatch_t	matches[nmatches];
	rc = regexec(&regex, pointspec.c_str(), nmatches, matches, 0);
	for (int i = 0; i < nmatches; i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d]: %d - %d", i,
			matches[i].rm_so, matches[i].rm_eo);
	}
	if (rc) {
		goto cleanup;
	}
	_x = std::stoi(pointspec.substr(matches[1].rm_so,
                        matches[1].rm_eo - matches[1].rm_so));
	_y = std::stoi(pointspec.substr(matches[2].rm_so,
                        matches[2].rm_eo - matches[2].rm_so));
cleanup:
	regfree(&regex);
	if (rc) {
		throw std::runtime_error("ImagePoint: no match");
	}
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
