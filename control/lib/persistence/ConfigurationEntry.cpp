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

ConfigurationEntry::ConfigurationEntry(const std::string& domain,
	const std::string& section,
	const std::string& name, const std::string& value)
	: ConfigurationKey(domain, section, name), _value(value) {
}

ConfigurationEntry::ConfigurationEntry(const ConfigurationKey& key,
	const std::string& value) {
	ConfigurationKey::operator=(key);
	_value = value;
}

ConfigurationEntry&	ConfigurationEntry::operator=(const ConfigurationEntry& other) {
	ConfigurationKey::operator=(other);
	_value = other.value();
	return *this;
}

bool	ConfigurationEntry::operator==(const ConfigurationEntry& other) const {
	return ConfigurationKey::operator==(other);
}

bool	ConfigurationEntry::operator<(const ConfigurationEntry& other) const {
	return ConfigurationKey::operator<(other);
}

} // namespace config
} // namespace astro
