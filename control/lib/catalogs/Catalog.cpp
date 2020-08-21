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

/**
 * \brief Find stars matching a name
 *
 * \param name		name prefix
 */
Catalog::starsetptr	Catalog::findLike(const std::string& name,
		size_t /* maxstars */) {
	Catalog::starsetptr	result(new starset());
	try {
		Star	s = find(name);
		result->insert(s);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "'%s' not found", name.c_str());
	}
	return result;
}

/**
 * \brief Convert a set of stars int a list
 *
 * \param stars		starset to convert
 */
std::set<std::string>	Catalog::starlist(const Catalog::starsetptr stars) {
	std::set<std::string>	result;
	std::set<Star>::const_iterator	i;
	for (i = stars->begin(); i != stars->end(); i++) {
		RaDec	pos = i->position(2000);
		std::string	s = stringprintf("%-20.20s|  %s %s",
					i->name().c_str(),
					pos.ra().hms(':', 1).substr(1).c_str(),
					pos.dec().dms(':', 0).c_str());
		result.insert(s);
	}
	return result;
}

/**
 * \brief Basic implementation of Tile retrieval
 *
 * This method is quite inefficient because it retrieves stars first
 * then turns them light weight stars. But it helps migration to fully
 * implemented light weight stars, because it makes tiles available in
 * all catalogs immediately.
 */
StarTilePtr	Catalog::findTile(const SkyWindow& window,
			const MagnitudeRange& magrange) {
	starsetptr	stars = find(window, magrange);
	StarTile	*tile = new StarTile(window, stars->size());
	// copy light weight stars
	std::copy(stars->begin(), stars->end(), tile->begin());
	return StarTilePtr(tile);
}

} // namespace catalog
} // namespace astro
