/*
 * Binning.cpp -- Binning implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <limits>
#include <astroregex.h>

namespace astro {
namespace image {

/**
 * \brief Construct a binning object
 */
Binning::Binning(int x, int y) : _x(x), _y(y) {
	if (x < 0) {
		throw std::range_error("x binning cannot be negative");
	}
	if (y < 0) {
		throw std::range_error("y binning cannot be negative");
	}
	if (_x == 0) { _x = 1; }
	if (_y == 0) { _y = 1; }
}

/**
 * \brief parse a Binning specification
 */
Binning::Binning(const std::string& binningspec) {
	std::string	r("\\(?([0-9]+)[,x]([0-9]+)\\)?");
	astro::regex	regex(r, astro::regex::extended);
	astro::smatch	matches;

	if (!regex_match(binningspec, matches, regex)) {
		std::string	msg = stringprintf("bad binning spec '%s'",
			binningspec.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	_x = std::stoi(matches[1]);
	_y = std::stoi(matches[2]);
}

/**
 * \brief Compare binning modes: equality
 *
 * This comparison does not take wildcards into account, i.e. if one
 * of the binning modes contains a wildcard, the other must contain
 * one at the same point
 */
bool	Binning::operator==(const Binning& other) const {
	return (_x == other._x) && (_y == other._y);
}

bool	Binning::operator!=(const Binning& other) const {
	return !operator==(other);
}

/**
 * \brief Compare binning modes: order
 *
 * This comparison should order the exact binning modes before the
 * wild card binning modes.
 */
bool	Binning::operator<(const Binning& other) const {
	if (_x < other._x) { return true; }
	if (_x > other._x) { return false; }
	// _x == other._x
	if (_y < other._y) { return true; }
	if (_y > other._y) { return false; }
	// _y == other._y
	return false;
}

/**
 * \brief Convert a Binning object into something printable
 */
std::string	Binning::toString() const {
	return stringprintf("(%ux%u)", _x, _y);
}

std::ostream&	operator<<(std::ostream& out, const Binning& binning) {
	return out << binning.toString();
}

std::istream&	operator>>(std::istream& in, Binning& binning) {
	char	c;
	int	x, y;
	in >> x >> c >> y;
	binning.x(x);
	binning.y(y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "binning mode parsed: %s",
		binning.toString().c_str());
	return in;
}

ImageSize       operator*(const ImageSize& size, const Binning& binning) {
	return ImageSize(size.width() * binning.x(),
			size.height() * binning.y());
}

ImageSize       operator/(const ImageSize& size, const Binning& binning) {
	return ImageSize(size.width() / binning.x(),
			size.height() / binning.y());
}

ImagePoint	operator*(const ImagePoint& point, const Binning& binning) {
	return ImagePoint(point.x() * binning.x(),
			point.y() * binning.y());
}

ImagePoint	operator/(const ImagePoint& point, const Binning& binning) {
	return ImagePoint(point.x() / binning.x(),
			point.y() / binning.y());
}

} // namespace image
} // namespace astro
