/*
 * ConfiguratioKey.cpp -- key object implemntation for configuration data
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>

namespace astro {
namespace config {

ConfigurationKey::ConfigurationKey() {
	_domain = "global";
}

ConfigurationKey::ConfigurationKey(const std::string& domain,
	const std::string& section, const std::string& name)
	: _domain(domain), _section(section), _name(name) {
}

ConfigurationKey::ConfigurationKey(const ConfigurationKey& other)
	: _domain(other.domain()), _section(other.section()),
	  _name(other.name()) {
}

ConfigurationKey&	ConfigurationKey::operator=(
				const ConfigurationKey& other) {
	_domain = other.domain();
	_section = other.section();
	_name = other.name();
	return *this;
}

bool	ConfigurationKey::operator==(const ConfigurationKey& other) const {
	return (_domain == other.domain()) && (_section == other.section())
		&& (_name == other.name());
}

bool	ConfigurationKey::operator<(const ConfigurationKey& other) const {
	if (domain() < other.domain()) {
		return true;
	}
	if (domain() > other.domain()) {
		return false;
	}
	if (section() < other.section()) {
		return true;
	}
	if (section() > other.section()) {
		return false;
	}
	if (name() < other.name()) {
		return true;
	}
	return false;
}

std::string	ConfigurationKey::condition() const {
	return stringprintf("domain = '%s' and section = '%s' and name = '%s'",
		domain().c_str(), section().c_str(), name().c_str());
}

std::string	ConfigurationKey::toString() const {
	return stringprintf("%s.%s.%s", domain().c_str(), section().c_str(),
		name().c_str());
}

} // namespace config
} // namespace astro
