/*
 * Ucac4ZoneIterator.cpp -- Ucac4 Zone iterator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace catalog {

Ucac4ZoneIterator::Ucac4ZoneIterator(Ucac4ZonePtr zone, const SkyWindow& window,
	const MagnitudeRange& magrange)
	: IteratorImplementation(true),
	  _zone(zone), _window(window), _magrange(magrange),
	  _predicate(_window, _magrange) {
	// does the zone actuall contain any stars from the window?
	if (!zone->touches(window)) {
		_isEnd = true;
	}

	// find out which index values we have to scan
	minindex = zone->first(window.leftra());
	maxindex = zone->first(window.rightra());
	if (minindex < maxindex) {
		_index = minindex;
	} else {
		_index = 1;
	}

	// call increment to ensure that the iterator points to the first star
	increment();
}

Ucac4ZoneIterator::~Ucac4ZoneIterator() {
}

Star    Ucac4ZoneIterator::operator*() {
	return *current_star;
}

bool    Ucac4ZoneIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

bool    Ucac4ZoneIterator::operator==(const Ucac4ZoneIterator& other) const {
	if (isEnd() != other.isEnd()) {
		return false;
	}
	if (_zone->zone() != other._zone->zone()) {
		return false;
	}
	return (_index == other._index);
}

std::string     Ucac4ZoneIterator::toString() const {
	return current_star->toString();
}

void    Ucac4ZoneIterator::increment() {
	while (!isEnd()) {
		_index++;

		// depending on the order of minindex and maxindex, we have
		// to skip some of the stars in a zone
		if (minindex < maxindex) {
			// in this case we scan from minindex to maxindex,
			// and we have to stop, when _index is larger than
			// maxindex
			if (_index > maxindex) {
				_isEnd = true;
				return;
			}
		} else { // maxindex < minindex
			// in this case we scan from 0 to maxindex, skip to
			// minindex, and continue scanning to the last star
			// in the zone
			if (_index > maxindex) {
				_index = minindex;
			}
			if (_index > _zone->numberOfStars()) {
				_isEnd = true;
				return;
			}
		}

		// get the star the iterator points to, and check the predicate
		Star	s = _zone->get(_index);
		if (_predicate(s)) {
			current_star = StarPtr(new Star(s));
			return;
		}
	}
}

} // namespace catalog
} // namespace astro
