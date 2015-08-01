/*
 * CatalogBackend.cpp -- catalog backend base class
 *
 * (c) 2014 Prof Dr Andreas Mueller
 */
#include "CatalogBackend.h"
#include <stdexcept>

namespace astro {
namespace catalog {

CatalogBackend::CatalogBackend() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "CatalogBackend constructor");
}

CatalogBackend::~CatalogBackend() {
}

Catalog::starsetptr	CatalogBackend::find(const SkyWindow& /* window */,
				const MagnitudeRange& /* magrange */) {
	throw std::runtime_error("find method must be overridden");
}

Star	CatalogBackend::find(const std::string& /* name */) {
	throw std::runtime_error("find method must be overridden");
}

} // namespace catalog
} // namespace astro
