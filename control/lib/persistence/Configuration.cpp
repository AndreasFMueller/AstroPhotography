/*
 * Configuration.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <cstdlib>
#include <GlobalTable.h>

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class ConfigurationBackend : public Configuration {
	std::string	_dbfilename;
	Database	_database;
	GlobalRecord	getglobal(const std::string& section,
				const std::string& name);
public:
	// constructor
	ConfigurationBackend(const std::string& filename);
	// global configuratoin variables
	virtual bool	hasglobal(const std::string& section,
				const std::string& name);
	virtual std::string	global(const std::string& section,
					const std::string& name);
	virtual std::string	global(const std::string& section,
					const std::string& name,
					const std::string& def);
	virtual void	setglobal(const std::string& section,
			const std::string& name, const std::string& value);
	virtual void	removeglobal(const std::string& name,
				const std::string& value);
	virtual std::list<ConfigurationEntry>	globallist();

	// get the configuration database
	virtual Database	database();
};

/**
 * \brief Construct a configuration backend
 */
ConfigurationBackend::ConfigurationBackend(const std::string& filename)
	: _dbfilename(filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", _dbfilename.c_str());
	_database = DatabaseFactory::get(_dbfilename);
}

//////////////////////////////////////////////////////////////////////
// global variable access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Get a global record from the global table
 */
GlobalRecord	ConfigurationBackend::getglobal(const std::string& section,
			const std::string& name) {
	GlobalTable	globals(_database);
	std::string	condition
		= stringprintf("section = '%s' and name = '%s'",
			_database->escape(section).c_str(),
			_database->escape(name).c_str());
	std::list<GlobalRecord>	records = globals.select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no variable for %s",
					condition.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NoSuchEntry("global", section, name);
	}
	return *records.begin();
}

/**
 * \brief Find out whether a given configuration value exists
 */
bool	ConfigurationBackend::hasglobal(const std::string& section,
		const std::string& name) {
	try {
		getglobal(section, name);
	} catch (const NoSuchEntry& x) {
		return false;
	}
	return true;
}

/**
 * \brief retrieve a global configuration variable
 */ 
std::string	ConfigurationBackend::global(const std::string& section,
			const std::string& name) {
	return getglobal(section, name).value;
}

/**
 * \brief retrieve a global configuration value, with a default
 *
 * If there is no configuration value for this section and name, then return
 * the default value
 */
std::string	ConfigurationBackend::global(const std::string& section,
			const std::string& name, const std::string& def) {
	try {
		return global(section, name);
	} catch (...) {
		return def;
	}
}

/*
 * \brief set a global configuration variable
 */
void	ConfigurationBackend::setglobal(const std::string& section,
		const std::string& name, const std::string& value) {
	GlobalTable	globals(_database);
	try {
		GlobalRecord	record = getglobal(section, name);
	} catch (...) {
		GlobalRecord	record;
		record.section = section;
		record.name = name;
		record.value = value;
		globals.add(record);
	}
}

/**
 * \brief remove a global configuration variable
 */
void	ConfigurationBackend::removeglobal(const std::string& section,
		const std::string& name) {
	GlobalTable	globals(_database);
	try {
		globals.remove(getglobal(section, name).id());
	} catch (...) {
	}
}

/**
 * \brief 
 */
std::list<ConfigurationEntry>	ConfigurationBackend::globallist() {
	GlobalTable	globals(_database);
	std::list<GlobalRecord>	records = globals.select("0 = 0");
	std::list<ConfigurationEntry>	result;
	std::list<GlobalRecord>::const_iterator	i;
	for (i = records.begin(); i != records.end(); i++) {
		ConfigurationEntry	entry;
		entry.section = i->section;
		entry.name = i->name;
		entry.value = i->value;
		result.push_back(entry);
	}
	return result;
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

//////////////////////////////////////////////////////////////////////
// implementation of the static methods of the Configuration fact
//////////////////////////////////////////////////////////////////////

typedef std::map<std::string, ConfigurationPtr>	configurationmap_t;
static configurationmap_t	configurationmap;

static std::string	configfilename() {
	std::string	filename;
	// get the path in the home directory
	char	*home = getenv("HOME");
	if (NULL != home) {
		filename = std::string(home) + "/.astrophoto.db";
	}

	// first check whether the environment variable is set
	char	*apconfig = getenv("AstroPhotoConfig");
	if (NULL != apconfig) {
		filename = std::string(apconfig);
	}
	return filename;
}

/**
 * \brief Get the configuration
 */
ConfigurationPtr	Configuration::get() {
	std::string	filename = get_default();

	// if the filename is still empty, then we have a problem
	debug(LOG_DEBUG, DEBUG_LOG, 0, "configuration file: %s",
		filename.c_str());
	if (0 == filename.size()) {
		throw std::runtime_error("no default config file name found");
	}
	return get(filename);
}

/**
 * \brief Get the configuration stored in a given database
 */
ConfigurationPtr	Configuration::get(const std::string& filename) {
	// check whether the configuration is already in the map
	configurationmap_t::iterator	i = configurationmap.find(filename);
	if (i != configurationmap.end()) {
		return i->second;
	}

	// we need to create a new configuration
	ConfigurationPtr	config(new ConfigurationBackend(filename));
	configurationmap.insert(std::make_pair(filename, config));
	return config;
}

static std::string	default_config;

/**
 * \brief get the default configuration filename
 */
std::string	Configuration::get_default() {
	if (0 == default_config.size()) {
		default_config = configfilename();
	}
	return default_config;
}

/**
 * \brief set the default filename
 */
void	Configuration::set_default(const std::string& filename) {
	default_config = filename;
}

} // namespace config
} // namespace astro
