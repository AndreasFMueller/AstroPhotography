/*
 * HipparcosIterator.cpp -- Hipparcos catalog iterator access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Hipparcos.h"
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

HipparcosIterator::HipparcosIterator(Hipparcos::starmap_t& stars)
	: IteratorImplementation(true), _stars(stars) {
	_i = _stars.begin();
}

Star	HipparcosIterator::operator*() {
	return _i->second;
}

bool	HipparcosIterator::operator==(const HipparcosIterator& other) const {
	return (_i == other._i);
}

bool	HipparcosIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	HipparcosIterator::increment() {
	if (isEnd()) {
		return;
	}
	++_i;
	_isEnd = (_i == _stars.end());
}

std::string	HipparcosIterator::toString() const {
	return stringprintf("%d", _i->first);
}

} // namespace catalog
} // namespace astro
