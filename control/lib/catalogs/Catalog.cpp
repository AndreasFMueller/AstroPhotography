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

/**
 * \brief function to perform precession on a starsetptr
 */
Catalog::starsetptr	precess(const Precession& precession,
				Catalog::starsetptr stars) {
	Catalog::starset	*precessedstars = new Catalog::starset();
	Catalog::starsetptr	result(precessedstars);
	
	Catalog::starset::const_iterator	i;
	for (i = stars->begin(); i != stars->end(); i++) {
		Star	s = *i;
		s.precess(precession);
		precessedstars->insert(s);
	}
	return result;
}

} // namespace catalog
} // namespace astro
