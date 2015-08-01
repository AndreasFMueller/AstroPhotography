/*
 * Catalog.cpp -- Catalog access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>
#include "CatalogBackend.h"
#include "CatalogIterator.h"
#include <typeinfo>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// Celestial Object implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Compute proper motion corrected position of an object
 */
RaDec	CelestialObject::position(const double epoch) const {
	RaDec	result;
	result.ra() = ra() + pm().ra() * epoch;
	result.dec() = dec() + pm().dec() * epoch;
	return result;
}

//////////////////////////////////////////////////////////////////////
// DeepSkyObject implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief string representation of a DeepSkyObject
 */
std::string	DeepSkyObject::toString() const {
	return stringprintf("%s: %s %s %.2f (%s)", name.c_str(),
			ra().hms().c_str(), dec().dms().c_str(), mag(),
			constellation.c_str());
}

//////////////////////////////////////////////////////////////////////
// MagnitudeRange implementation
//////////////////////////////////////////////////////////////////////
std::string	MagnitudeRange::toString() const {
	return stringprintf("[%.2f, %.2f]", brightest(), faintest());
}


//////////////////////////////////////////////////////////////////////
// Catalog implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Destructor
 */
Catalog::~Catalog() {
}

/**
 * \brief All end iterators use the same GenericEndIteratorImplementation
 */
CatalogIterator	Catalog::end() {
	IteratorImplementationPtr	impl(
		new GenericEndIteratorImplementation());
	return CatalogIterator(impl);
}

/**
 * \brief Default window selection iterator
 *
 * This implementation uses a condition iterator on the standard iterator
 * with a WindowPredicate. This will always work, but will be very inefficient
 * with large catalogs.
 */
CatalogIterator	Catalog::findIter(const SkyWindow& window,
		const MagnitudeRange& magrange) {
	IteratorPredicatePtr	predicate(
		new WindowPredicate(window, magrange));
	IteratorImplementationPtr       impl(new ConditionIterator(
		begin().implementation(), predicate));
	return CatalogIterator(impl);
}

/**
 * \brief Dummy implementation of begin()
 */
CatalogIterator	Catalog::begin() {
	std::string	msg = stringprintf("%s::begin() not implemented",
				typeid(*this).name());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::logic_error(msg);
}

} // namespace catalog
} // namespace astro
