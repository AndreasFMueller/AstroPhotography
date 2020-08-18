/*
 * MagnitudeRange.cpp -- MagnitudeRange implementation
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

std::string	MagnitudeRange::toString() const {
	return stringprintf("[%.2f, %.2f]", brightest(), faintest());
}

} // namespace catalog
} // namespace astro
