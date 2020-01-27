/*
 * SAOIterator.cpp -- SAO catalog iterator access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "SAO.h"
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

SAOIterator::SAOIterator(SAO::starmap_t& stars)
	: IteratorImplementation(true), _stars(stars) {
	_i = _stars.begin();
}

Star	SAOIterator::operator*() {
	return _i->second;
}

bool	SAOIterator::operator==(const SAOIterator& other) const {
	return (_i == other._i);
}

bool	SAOIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	SAOIterator::increment() {
	if (isEnd()) {
		return;
	}
	++_i;
	_isEnd = (_i == _stars.end());
}

std::string	SAOIterator::toString() const {
	return stringprintf("%d", _i->first);
}

} // namespace catalog
} // namespace astro
