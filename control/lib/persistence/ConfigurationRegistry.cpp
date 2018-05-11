/*
 * ConfigurationRegistry.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ConfigurationRegistry.h"

namespace astro {
namespace config {

/**
 * \brief list all registered keys
 */
std::list<ConfigurationKey>	ConfigurationRegistry::list() const {
	std::list<ConfigurationKey>	result;
	descriptions_t::const_iterator	i;
	for (i = _descriptions.begin(); i != _descriptions.end(); i++) {
		result.push_back(i->first);
	}
	return result;
}

/**
 * \brief Remember a key and description
 */
void    ConfigurationRegistry::add(const ConfigurationKey& key, 
                        const std::string& description) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s", key.toString().c_str(),
		description.c_str());
	_descriptions.insert(std::make_pair(key, description));
}

/**
 * \brief Get description for a key
 */
const std::string&	ConfigurationRegistry::describe(
				const ConfigurationKey& key) const {
	descriptions_t::const_iterator	i = _descriptions.find(key);
	if (_descriptions.end() == i) {
		throw astro::NotFound("key " + key.toString()
				+ " does not exist");
	}
	return i->second;
}

/**
 * \brief Show the registry
 */
void	ConfigurationRegistry::show(std::ostream& out,
		bool showdescriptions) const {
	descriptions_t::const_iterator	i;
	for (i = _descriptions.begin(); i != _descriptions.end(); i++) {
		out << i->first.toString();
		if (showdescriptions) {
			out << " ";
			out << i->second;
		}
		out << std::endl;
	}
}

} // namespace config
} // namespace astro
