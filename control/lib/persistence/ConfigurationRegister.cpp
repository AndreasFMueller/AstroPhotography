/*
 * ConfigurationRegister.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroDebug.h>

namespace astro {
namespace config {

ConfigurationRegister::ConfigurationRegister(const std::string& domain,
	const std::string& section, const std::string& name,
	const std::string& description)
	: ConfigurationKey(domain, section, name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering %s: %s",
		toString().c_str(), description.c_str());
	Configuration::registerkey(*this, description);
}

ConfigurationRegister::ConfigurationRegister(const ConfigurationKey& key,
	const std::string& description)
	: ConfigurationKey(key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering %s: %s",
		toString().c_str(), description.c_str());
	Configuration::registerkey(*this, description);
}

} // namespace config
} // namespace astro
