/*
 * Binning.cpp -- Binning implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>

#include <stdexcept>

namespace astro {
namespace camera {

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
 * \brief compare binning modes: compatibility
 *
 * This comparisaon takes wildcards into account.
 */
bool	Binning::compatible(const Binning& other) const {
	return	((x == other.x) || (x == -1) || (other.x == -1)) &&
		((y == other.y) || (y == -1) || (other.y == -1));
}

/**
 * \brief Test whether the binning mode has any wildcards.
 */
bool	Binning::iswildcard() const {
	return (x == -1) || (y == -1);
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
	push_back(Binning());
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
	for (std::vector<Binning>::const_iterator i = begin(); i != end(); i++) {
		if (i->compatible(binning)) {
			return true;
		}
	}
	return false;
}

} // namespace camera
} // namespace astro
