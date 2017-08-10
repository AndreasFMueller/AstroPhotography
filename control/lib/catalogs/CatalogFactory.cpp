/*
 * CatalogFactory.cpp -- build a catalog without exposing the catalog classes
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <Hipparcos.h>
#include <BSC.h>
#include <Tycho2.h>
#include <Ucac4.h>
#include <CatalogBackend.h>

namespace astro {
namespace catalog {

CatalogPtr	CatalogFactory::get(BackendType type,
			const std::string& parameter) {
	switch (type) {
	case BSC:
		return CatalogPtr(new catalog::BSC(parameter));
	case Hipparcos:
		return CatalogPtr(new catalog::Hipparcos(parameter));
	case Tycho2:
		return CatalogPtr(new catalog::Tycho2(parameter));
	case Ucac4:
		return CatalogPtr(new catalog::Ucac4(parameter));
	case Combined:
		return CatalogPtr(new FileBackend(parameter));
	case Database:
		return CatalogPtr(new DatabaseBackend(parameter));;
	}
	throw std::runtime_error("unknown catalog");
}

} // namespace catalog
} // namespace astro
