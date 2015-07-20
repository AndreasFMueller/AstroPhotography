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

using namespace astro::image;

namespace astro {
namespace camera {

/**
 * \brief Construct a binning object
 */
Binning::Binning(unsigned int x, unsigned int y) : _x(x), _y(y) {
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
	unsigned int	x, y;
	in >> x >> c >> y;
	binning.x(x);
	binning.y(y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "binning mode parsed: %s",
		binning.toString().c_str());
	return in;
}

// The binningtester class is used as a functor 
/**
 * \brief Binning mode compatibility tester
 *
 * This is an auxiliary class usedby the BinningSet::permits() method
 * to compatibility test all binning modes of a set using an STL algorithm.
 */
class binningtester {
	Binning	binning;
public:
	binningtester(const Binning& _binning) : binning(_binning) { }
	bool	operator()(const Binning& b) const {
		return b == binning;
	}
};

/**
 * \brief Construction of a binning set
 *
 * This constructor is needed because the BinningSet should always
 * contain at least the 1x1 binning mode.
 */
BinningSet::BinningSet() {
	insert(Binning());
}

/**
 * \brief Test whether a binning mode is allowed by a set of binning modes
 *
 * \param binning	binning mode to test for compatibility. This mode
 *			may not have any wildcards.
 * \throws std::range_error	is thrown if the binning mode contains
 *				wildcards
 */
bool	BinningSet::permits(const Binning& binning) const throw (std::range_error) {
	BinningSet::const_iterator	i = find(binning);
	return (end() != i);
}

std::string	BinningSet::toString() const {
	std::string	result;
	std::set<Binning>::const_iterator	i;
	for (i = begin(); i != end(); i++) {
		if (i != begin()) {
			result.append(",");
		}
		result.append(i->toString());
	}
	return result;
}

std::ostream&	operator<<(std::ostream& out, const BinningSet& binningset) {
	return out << binningset.toString();
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


} // namespace camera
} // namespace astro
