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

namespace astro {
namespace camera {

const unsigned int	Binning::wildcard = std::numeric_limits<unsigned int>::max();

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

/**
 * \brief Compare binning modes: order
 *
 * This comparison should order the exact binning modes before the
 * wild card binning modes.
 */
int	Binning::operator<(const Binning& other) const {
	if (x < other.x) { return 1; }
	if (x > other.x) { return -1; }
	// x == other.x
	if (y < other.y) { return -1; }
	if (y > other.y) { return 1; }
	// y == other.y
	return 0;
}

/**
 * \brief compare binning modes: compatibility
 *
 * This comparisaon takes wildcards into account.
 */
bool	Binning::compatible(const Binning& other) const {
	return	((x == other.x) || (x == wildcard) || (other.x == wildcard)) &&
		((y == other.y) || (y == wildcard) || (other.y == wildcard));
}

/**
 * \brief Convert a Binning object into something printable
 */
std::string	Binning::toString() const {
	if (isXwildcard() && isYwildcard()) {
		return std::string("(*x*)");
	}
	if (isXwildcard()) {
		return stringprintf("(*x%u)", y);
	}
	if (isYwildcard()) {
		return stringprintf("(%ux*)", x);
	}
	return stringprintf("(%ux%u)", x, y);
}

std::ostream&	operator<<(std::ostream& out, const Binning& binning) {
	return out << binning.toString();
}

/**
 * \brief Test whether the binning mode has any wildcards.
 */
bool	Binning::iswildcard() const {
	return (x == wildcard) || (y == wildcard);
}

/**
 * \brief Test whether the binning mode has any wildcards.
 */
bool	Binning::isXwildcard() const {
	return (x == wildcard);
}

/**
 * \brief Test whether the binning mode has any wildcards.
 */
bool	Binning::isYwildcard() const {
	return (y == wildcard);
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
	if (binning.iswildcard()) {
		throw std::range_error("invalid binning type");
	}
	for (std::set<Binning>::const_iterator i = begin(); i != end(); i++) {
		if (i->compatible(binning)) {
			return true;
		}
	}
	return false;
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

} // namespace camera
} // namespace astro
