/*
 * ConfiguratioKey.cpp -- key object implemntation for configuration data
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>

namespace astro {
namespace config {

ConfigurationKey::ConfigurationKey() {
	domain = "global";
}

ConfigurationKey::ConfigurationKey(const std::string& _domain,
	const std::string& _section, const std::string& _name)
	: domain(_domain), section(_section), name(_name) {
}

ConfigurationKey::ConfigurationKey(const ConfigurationKey& other)
	: domain(other.domain), section(other.section), name(other.name) {
}

ConfigurationKey&	ConfigurationKey::operator=(
				const ConfigurationKey& other) {
	domain = other.domain;
	section = other.section;
	name = other.name;
	return *this;
}

bool	ConfigurationKey::operator==(const ConfigurationKey& other) const {
	return (domain == other.domain) && (section == other.section)
		&& (name == other.name);
}

bool	ConfigurationKey::operator<(const ConfigurationKey& other) const {
	if (domain < other.domain) {
		return true;
	}
	if (domain > other.domain) {
		return false;
	}
	if (section < other.section) {
		return true;
	}
	if (section > other.section) {
		return false;
	}
	if (name < other.name) {
		return true;
	}
	return false;
}

std::string	ConfigurationKey::condition() const {
	return stringprintf("domain = '%s' and section = '%s' and name = '%s'",
		domain.c_str(), section.c_str(), name.c_str());
}

std::string	ConfigurationKey::toString() const {
	return stringprintf("%s.%s.%s", domain.c_str(), section.c_str(),
		name.c_str());
}

} // namespace config
} // namespace astro
