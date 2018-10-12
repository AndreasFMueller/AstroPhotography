/*
 * DeepSkyCatalogFactory.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include "DeepSkyCatalogs.h"

namespace astro {
namespace catalog {

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
	DeepSkyCatalog	*catalog = NULL;
	switch (catalogtype) {
	case Messier:
		catalog = new MessierCatalog(_basedir + "/messier");
		break;
	case NGCIC:
		catalog = new NGCICCatalog(_basedir + "/ngcic");
		break;
	default:
		throw std::runtime_error("unknown deep sky catalog");
	}
	return DeepSkyCatalogPtr(catalog);
}

} // namespace catalog
} // namespace astro
