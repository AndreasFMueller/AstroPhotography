/*
 * BinningSet.cpp -- Binning implementation
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <limits>

using namespace astro::image;

namespace astro {
namespace camera {

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
bool	BinningSet::permits(const Binning& binning) const {
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

} // namespace camera
} // namespace astro
