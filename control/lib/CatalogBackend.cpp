/*
 * CatalogBackend.cpp -- catalog backend base class
 *
 * (c) 2014 Prof Dr Andreas Mueller
 */
#include <CatalogBackend.h>
#include <stdexcept>

namespace astro {
namespace catalog {

CatalogBackend::CatalogBackend() {
}

CatalogBackend::~CatalogBackend() {
}

Catalog::starsetptr	find(const SkyWindow& window, double minimum_magnitude) {
	throw std::runtime_error("find method must be overridden");
}

} // namespace catalog
} // namespace astro
