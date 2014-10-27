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
#include <regex>

using namespace astro::image;

namespace astro {
namespace camera {

/**
 * \brief Construct a binning object
 */
Binning::Binning(unsigned int _x, unsigned int _y) : x(_x), y(_y) {
	if (x == 0) { x = 1; }
	if (y == 0) { y = 1; }
}

/**
 * \brief parse a Binning specification
 */
Binning::Binning(const std::string& binningspec) {
	std::string	r("\\(?([0-9]+)[,x]([0-9]+)\\)?");
	std::regex	regex(r, std::regex::extended);
	std::smatch	matches;

	if (!std::regex_match(binningspec, matches, regex)) {
		std::string	msg = stringprintf("bad binning spec '%s'",
			binningspec.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	x = std::stoi(matches[1]);
	y = std::stoi(matches[2]);
}

/**
 * \brief Compare binning modes: equality
 *
 * This comparison does not take wildcards into account, i.e. if one
 * of the binning modes contains a wildcard, the other must contain
 * one at the same point
 */
bool	Binning::operator==(const Binning& other) const {
	return (x == other.x) && (y == other.y);
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
	if (x < other.x) { return true; }
	if (x > other.x) { return false; }
	// x == other.x
	if (y < other.y) { return true; }
	if (y > other.y) { return false; }
	// y == other.y
	return false;
}

/**
 * \brief Convert a Binning object into something printable
 */
std::string	Binning::toString() const {
	return stringprintf("(%ux%u)", x, y);
}

std::ostream&	operator<<(std::ostream& out, const Binning& binning) {
	return out << binning.toString();
}

std::istream&	operator>>(std::istream& in, Binning& binning) {
	char	c;
	unsigned int	x, y;
	in >> x >> c >> y;
	binning.setX(x);
	binning.setY(y);
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
	return ImageSize(size.width() * binning.getX(),
			size.height() * binning.getY());
}

ImageSize       operator/(const ImageSize& size, const Binning& binning) {
	return ImageSize(size.width() / binning.getX(),
			size.height() / binning.getY());
}

ImagePoint	operator*(const ImagePoint& point, const Binning& binning) {
	return ImagePoint(point.x() * binning.getX(),
			point.y() * binning.getY());
}

ImagePoint	operator/(const ImagePoint& point, const Binning& binning) {
	return ImagePoint(point.x() / binning.getX(),
			point.y() / binning.getY());
}


} // namespace camera
} // namespace astro
