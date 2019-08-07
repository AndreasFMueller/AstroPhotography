/*
 * MessierCatalog.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "DeepSkyCatalogs.h"

namespace astro {
namespace catalog {

/**
 * \brief Empty Messier catalog implementation
 */
DeepSkyObjectSetPtr     MessierCatalog::find(const SkyWindow&) {
	DeepSkyObjectSetPtr	result;
	return result;
}

DeepSkyObject	MessierCatalog::find(const std::string& /* name */) {
	throw std::runtime_error("find by name not implemented yet");
}

std::set<std::string>	MessierCatalog::findLike(const std::string& /* name */) {
	throw std::runtime_error("findLike not implemented");
}

} // namespace catalog
} // namespace astro
