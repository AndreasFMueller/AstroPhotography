/*
 * ConfiguratioEntry.cpp -- entry object implementation for configuration data
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>

namespace astro {
namespace config {

ConfigurationEntry::ConfigurationEntry() {
}

ConfigurationEntry::ConfigurationEntry(const std::string& _domain,
	const std::string& _section,
	const std::string& _name, const std::string& _value)
	: ConfigurationKey(_domain, _section, _name), value(_value) {
}

ConfigurationEntry::ConfigurationEntry(const ConfigurationKey& key,
	const std::string& _value) {
	ConfigurationKey::operator=(key);
	value = _value;
}

bool	ConfigurationEntry::operator==(const ConfigurationEntry& other) const {
	return ConfigurationKey::operator==(other);
}

bool	ConfigurationEntry::operator<(const ConfigurationEntry& other) const {
	return ConfigurationKey::operator<(other);
}

} // namespace config
} // namespace astro
