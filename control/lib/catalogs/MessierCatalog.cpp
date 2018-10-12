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
DeepSkyCatalog::deepskyobjectsetptr     MessierCatalog::find(const SkyWindow&) {
	deepskyobjectsetptr	result;
	return result;
}

DeepSkyObject	MessierCatalog::find(const std::string& name) {
	throw std::runtime_error("find by name not implemented yet");
}

} // namespace catalog
} // namespace astro
