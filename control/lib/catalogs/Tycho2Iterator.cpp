/*
 * Tycho2Iterator.cpp -- Tycho2 star catalog class declarations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Tycho2.h"
#include <includes.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <limits>

namespace astro {
namespace catalog {

Tycho2Iterator::Tycho2Iterator(unsigned int index, Tycho2& catalog) 
	: IteratorImplementation(true), _index(index), _catalog(catalog) {
	_isEnd = (_index >= _catalog.numberOfStars());
}

Tycho2Iterator::~Tycho2Iterator() {
}

Star 	Tycho2Iterator::operator*() {
	if (isEnd()) {
		throw std::logic_error("cannot dereference end iterator");
	}
	return _catalog.find(_index);
}

bool	Tycho2Iterator::operator==(const Tycho2Iterator& other) const {
	if (isEnd() != other.isEnd()) {
		return false;
	}
	return (_index == other._index);
}

bool	Tycho2Iterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	Tycho2Iterator::increment() {
	if (isEnd()) {
		return;
	}
	++_index;
	if (_index >= _catalog.numberOfStars()) {
		_index = _catalog.numberOfStars();
		_isEnd = true;
	}
}

std::string	Tycho2Iterator::toString() const {
	return stringprintf("%d", _index);
}

} // namespace catalog
} // namespace astro
