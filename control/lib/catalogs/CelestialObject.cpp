/*
 * CelestialObject.cpp -- Celestial object implementation
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
 * \brief Compute proper motion corrected position of an object
 */
RaDec	CelestialObject::position(const double epoch) const {
	RaDec	result;
	result.ra() = ra() + pm().ra() * epoch;
	result.dec() = dec() + pm().dec() * epoch;
	return result;
}

} // namespace catalog
} // namespace astro
