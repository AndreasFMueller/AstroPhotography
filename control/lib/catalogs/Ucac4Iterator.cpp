/*
 * Ucac4Iterator.cpp -- Ucac4Iterator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

Ucac4Iterator::Ucac4Iterator(uint16_t zone, uint32_t index, Ucac4& catalog)
	: IteratorImplementation(!(zone > 900)),
	  _zone(zone), _index(index), _catalog(catalog) {
	if (_zone > 900) {
		_index = 0;
	}
}

Ucac4Iterator::~Ucac4Iterator() {
}

Star	Ucac4Iterator::operator*() {
	return _catalog.find(Ucac4StarNumber(_zone, _index));
}

bool	Ucac4Iterator::operator==(const Ucac4Iterator& other) const {
	return (_zone == other._zone) && (_index == other._index);
}

bool	Ucac4Iterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

void	Ucac4Iterator::increment() {
	if (isEnd()) {
		return;
	}
	++_index;
	if (_index >= _catalog.getzone(_zone)->numberOfStars()) {
		++_zone;
		if (_zone > 900) {
			_zone = 901;
			_isEnd = true;
		}
		_index = 0;
	}
}

std::string	Ucac4Iterator::toString() const {
	return Ucac4StarNumber(_zone, _index).toString();
}

} // namespace catalog
} // namespace astro
