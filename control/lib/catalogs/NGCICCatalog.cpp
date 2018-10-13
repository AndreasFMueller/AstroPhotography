/*
 * NGCICCatalog.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "DeepSkyCatalogs.h"

namespace astro {
namespace catalog {

/**
 * \brief Construct the NGCIC catalog
 */
NGCICCatalog::NGCICCatalog(const std::string& path)
	: DeepSkyCatalog(path), NGCIC(path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "NGCIC with %d objects", size());
}

/**
 * \brief Empty NGCIC catalog implementation
 */
DeepSkyCatalog::deepskyobjectsetptr     NGCICCatalog::find(const SkyWindow& window) {
	return NGCIC::find(window);
}


DeepSkyObject	NGCICCatalog::find(const std::string& name) {
	return NGCIC::find(name);
}

std::set<std::string>	NGCICCatalog::findLike(const std::string& name) {
	return NGCIC::findLike(name);
}

} // namespace catalog
} // namespace astro
