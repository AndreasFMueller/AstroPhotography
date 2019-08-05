/*
 * PGCCatalog.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "DeepSkyCatalogs.h"

namespace astro {
namespace catalog {

/**
 * \brief Construct the PGC catalog
 */
PGCCatalog::PGCCatalog(const std::string& path)
	: DeepSkyCatalog(path), PGC(path) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "PGC with %d objects", size());
}

/**
 * \brief Empty PGC catalog implementation
 */
DeepSkyCatalog::deepskyobjectsetptr     PGCCatalog::find(const SkyWindow& window) {
	return PGC::find(window);
}


DeepSkyObject	PGCCatalog::find(const std::string& name) {
	return PGC::find(name);
}

std::set<std::string>	PGCCatalog::findLike(const std::string& name) {
	return PGC::findLike(name);
}

} // namespace catalog
} // namespace astro
