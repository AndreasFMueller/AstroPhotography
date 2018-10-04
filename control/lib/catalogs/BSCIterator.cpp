/*
 * BSCIterator.cpp -- bright star catalog iterator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "BSC.h"
#include <fstream>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroFormat.h>
#include <typeinfo>

namespace astro {
namespace catalog {

BSCIterator::BSCIterator(BSC::starmap_t& stars)
	: IteratorImplementation(true), _stars(stars) {
	_i = _stars.begin();
}

Star	BSCIterator::operator*() {
	return _i->second;
}

bool	BSCIterator::operator==(const BSCIterator& other) const {
	return (_i == other._i);
}

bool	BSCIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	BSCIterator::increment() {
	if (_isEnd) {
		return;
	}
	++_i;
	_isEnd = (_i == _stars.end());
}

std::string	BSCIterator::toString() const {
	return stringprintf("%d", _i->first);
}

} // namespace catalog
} // namespace astro
