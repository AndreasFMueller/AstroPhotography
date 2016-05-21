/*
 * ConfigurationBackend.cpp -- backend implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ConfigurationBackend.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief Construct a configuration backend
 */
ConfigurationBackend::ConfigurationBackend(const std::string& filename)
	: _dbfilename(filename), _database(DatabaseFactory::get(_dbfilename)),
	  _configurationtable(_database) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", _dbfilename.c_str());
}

bool    ConfigurationBackend::has(const ConfigurationKey& key) {
	return _configurationtable.has(key.condition());
}

bool    ConfigurationBackend::has(const std::string& domain,
                                const std::string& section,
                                const std::string& name) {
	return has(ConfigurationKey(domain, section, name));
}

std::string	ConfigurationBackend::get(const ConfigurationKey& key) {
	return _configurationtable[key].value;
}

std::string     ConfigurationBackend::get(const std::string& domain,
                                        const std::string& section,
                                        const std::string& name) {
	return get(ConfigurationKey(domain, section, name));
}

std::string	ConfigurationBackend::get(const ConfigurationKey& key,
			const std::string& def) {
	try {
		return get(key);
	} catch (const NoSuchEntry& x) {
	}
	return def;
}

std::string     ConfigurationBackend::get(const std::string& domain,
                                        const std::string& section,
                                        const std::string& name,
                                        const std::string& def) {
	return get(ConfigurationKey(domain, section, name), def);
}

void    ConfigurationBackend::set(const std::string& domain,
		const std::string& section, const std::string& name,
		const std::string& value) {
	ConfigurationKey	key(domain, section, name);
	if (has(key)) {
		long	id = _configurationtable.key2id(key);
		ConfigurationEntry	entry = _configurationtable.byid(id);
		entry.value = value;
		_configurationtable.update(id, entry);
	} else {
		ConfigurationEntry	entry(key, value);
		_configurationtable.add(entry);
	}
}

void    ConfigurationBackend::remove(const std::string& domain,
		const std::string& section, const std::string& name) {
	_configurationtable.remove(domain, section, name);
}

void	ConfigurationBackend::remove(const ConfigurationKey& key) {
	_configurationtable.remove(key);
}

std::list<ConfigurationEntry>   ConfigurationBackend::list() {
	return _configurationtable.selectAll();
}

std::list<ConfigurationEntry>   ConfigurationBackend::list(
		const std::string& domain) {
	return _configurationtable.selectDomain(domain);
}

std::list<ConfigurationEntry>   ConfigurationBackend::list(
		const std::string& domain, const std::string& section) {
	return _configurationtable.selectSection(domain, section);
}

//////////////////////////////////////////////////////////////////////
// database method implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief access to the database
 */
Database	ConfigurationBackend::database() {
	return _database;
}

} // namespace config
} // namespace astro
