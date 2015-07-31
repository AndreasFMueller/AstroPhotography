/*
 * Ucac4WindowIterator.cpp 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Ucac4.h"

namespace astro {
namespace catalog {

Ucac4WindowIterator::Ucac4WindowIterator(Ucac4& catalog,
	const SkyWindow& window, const MagnitudeRange magrange)
	: IteratorImplementation(true),
	  _catalog(catalog), _window(window), _magrange(magrange) {
	// find out which zones we have to include in the scan
	zoneinterval = Ucac4::zoneinterval(window);

	// get the first zone
	uint16_t	zonenumber = zoneinterval.first;
	do {
		Ucac4ZonePtr	zoneptr = _catalog.getzone(zonenumber);
		zoneiterator = Ucac4ZoneIteratorPtr(
			new Ucac4ZoneIterator(zoneptr, _window, _magrange)
		);
		if (!zoneiterator->isEnd()) {
			return;
		}
		zonenumber++;
	} while (zonenumber <= zoneinterval.second);
	_isEnd = true;
}

Ucac4WindowIterator::~Ucac4WindowIterator() {
}

Star    Ucac4WindowIterator::operator*() {
	return zoneiterator->operator*();
}

bool    Ucac4WindowIterator::operator==(const Ucac4WindowIterator& other) const {
	if (isEnd() != other.isEnd()) {
		return false;
	}
	if (zoneiterator->zone() != other.zoneiterator->zone()) {
		return false;
	}
	return (zoneiterator != other.zoneiterator);
}

bool    Ucac4WindowIterator::operator==(const IteratorImplementation& other) const {
	return equal_implementation(this, other);
}

std::string     Ucac4WindowIterator::toString() const {
	return zoneiterator->toString();
}

void    Ucac4WindowIterator::increment() {
	while (!isEnd()) {
		// advance the iterator
		zoneiterator->increment();

		// if the iterator is not at the end, we can stop here
		if (!zoneiterator->isEnd()) {
			return;
		}

		// if the iterator ends, we have to jump to the next zone
		uint16_t	zonenumber = zoneiterator->zone() + 1;
		if (zonenumber > zoneinterval.second) {
			_isEnd = true;
			return;
		}
		Ucac4ZonePtr	zoneptr = _catalog.getzone(zonenumber);
		zoneiterator = Ucac4ZoneIteratorPtr(
			new Ucac4ZoneIterator(zoneptr, _window, _magrange)
		);

		// if the zoneiterator is not pointing to the end, we were
		// successful
		if (!zoneiterator->isEnd()) {
			return;
		}
	}
}

} // namespace catalog
} // namespace astro
