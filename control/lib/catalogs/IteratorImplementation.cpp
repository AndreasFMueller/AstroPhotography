/*
 * IteratorImplementation.cpp -- implementation file for Iterators
 *
 * We mainly need this so that we can implement the virtual destructor,
 * and thus ensure there is a well defined location for the virtual function
 * table for the IteratorImplementation class. This in return seems to
 * be necessary for typeinfo.
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CatalogIterator.h"

namespace astro {
namespace catalog {

IteratorImplementation::IteratorImplementation(bool begin_or_end)
	: _isEnd(!begin_or_end) {
}

IteratorImplementation::~IteratorImplementation() {
}

bool	IteratorImplementation::operator!=(const IteratorImplementation& other)
		const {
	return !(operator==(other));
}

} // namespace catalog
} // namespace astro
