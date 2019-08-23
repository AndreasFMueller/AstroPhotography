/*
 * DeepSkyCatalogFactory.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include "DeepSkyCatalogs.h"

namespace astro {
namespace catalog {

std::map<DeepSkyCatalogFactory::deepskycatalog_t, DeepSkyCatalogPtr>	DeepSkyCatalogFactory::catalogmap;

/**
 * \brief Construct a catalog based on the directory
 */
DeepSkyCatalogFactory::DeepSkyCatalogFactory()
	: _basedir(DATAROOTDIR "/starcatalogs") {
}

/**
 * \brief construct a catalog of a given type
 */
DeepSkyCatalogPtr	DeepSkyCatalogFactory::get(deepskycatalog_t catalogtype) {
	DeepSkyCatalogPtr	catalog;
	auto	i = catalogmap.find(catalogtype);
	if (i != catalogmap.end()) {
		return i->second;
	}
	switch (catalogtype) {
	case Messier:
		catalog = DeepSkyCatalogPtr(
				new MessierCatalog(_basedir + "/messier"));
		break;
	case NGCIC:
		catalog = DeepSkyCatalogPtr(
				new NGCICCatalog(_basedir + "/ngcic"));
		break;
	case PGC:
		catalog = DeepSkyCatalogPtr(
				new PGCCatalog(_basedir + "/pgc"));
		break;
	default:
		throw std::runtime_error("unknown deep sky catalog");
	}
	catalogmap.insert(std::make_pair(catalogtype, catalog));
	return catalog;
}

} // namespace catalog
} // namespace astro
