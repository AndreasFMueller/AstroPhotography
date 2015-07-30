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

void	CatalogBackend::add(int id, const Star& star) {
	throw std::runtime_error("add not possible in this typo of backend");
}

} // namespace catalog
} // namespace astro
