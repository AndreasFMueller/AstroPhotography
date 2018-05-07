/*
 * ConfigurationBackend.cpp -- backend implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ConfigurationBackend.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

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

void	ConfigurationBackend::set(const ConfigurationKey& key,
	const std::string& value) {
	set(key.domain, key.section, key.name, value);
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

/**
 * \brief Set the path for the media database
 */
void	ConfigurationBackend::setMediaPath(const std::string& path) {
	// make sure the path actually is a directory
	struct stat	sb;
	if (stat(path.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat %s: %s",
			path.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (!S_ISDIR(sb.st_mode)) {
		std::string	msg = stringprintf("%s is not a directory",
			path.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	if (!access(path.c_str(), W_OK)) {
		std::string	msg = stringprintf("cannot write %s",
			path.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "'%s' is suitable as media path");

	set("system", "configuration", "media", path);
}

/**
 * \brief Get the media path
 */
std::string	ConfigurationBackend::getMediaPath() {
	if (!has("system", "configuration", "media")) {
		return std::string("");
	}
	return get("system", "configuration", "media");
}

/**
 * \brief Get the media database
 */
Database	ConfigurationBackend::mediadatabase() {
	// ask the configuration database for the key named
	if (has("system", "configuration", "media")) {
		return _database;
	}

	// try alternative Media database path
	std::string	_mediadbfilename = get("system", "configuration",
				"media", "") + "/media.db";
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening media database '%s'",
		_mediadbfilename.c_str());
	
	// get the database based on this filename
	return DatabaseFactory::get(_mediadbfilename);
}

} // namespace config
} // namespace astro
