/*
 * DeepSkyObject.cpp -- DeepSkyObject implementation
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

} // namespace catalog
} // namespace astro
