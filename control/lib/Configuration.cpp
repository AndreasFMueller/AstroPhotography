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

namespace astro {
namespace config {

/**
 * \brief configuration backend
 *
 * This is used to hide the fact that there 
 */
class ConfigurationBackend : public Configuration {
	std::string	dbfilename;
	Database	database;
	GlobalRecord	getglobal(const std::string& section,
				const std::string& name);
public:
	// constructor
	ConfigurationBackend(const std::string& filename);
	// global configuratoin variables
	virtual std::string	global(const std::string& section,
					const std::string& name);
	virtual std::string	global(const std::string& section,
					const std::string& name,
					const std::string& def);
	virtual void	setglobal(const std::string& section,
			const std::string& name, const std::string& value);
	virtual void	removeglobal(const std::string& name,
				const std::string& value);
	virtual DeviceMapperPtr	devicemapper();
};

/**
 * \brief Construct a configuration backend
 */
ConfigurationBackend::ConfigurationBackend(const std::string& filename)
	: dbfilename(filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", dbfilename.c_str());
	database = DatabaseFactory::get(dbfilename);
}

/**
 * \brief Get a global record from the global table
 */
GlobalRecord	ConfigurationBackend::getglobal(const std::string& section,
			const std::string& name) {
	GlobalTable	globals(database);
	std::string	condition
		= stringprintf("section = '%s' and name = '%s'",
			database->escape(section).c_str(),
			database->escape(name).c_str());
	std::list<GlobalRecord>	records = globals.select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no variable for %s",
					condition.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return *records.begin();
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
	GlobalTable	globals(database);
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
	GlobalTable	globals(database);
	try {
		globals.remove(getglobal(section, name).id());
	} catch (...) {
	}
}

//////////////////////////////////////////////////////////////////////
// implementation of the static methods of the Configuration fact
//////////////////////////////////////////////////////////////////////

typedef std::map<std::string, ConfigurationPtr>	configurationmap_t;
static configurationmap_t	configurationmap;

/**
 * \brief Get the configuration
 */
ConfigurationPtr	Configuration::get() {
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

/**
 * \brief Get the device mapper
 */
DeviceMapperPtr	ConfigurationBackend::devicemapper() {
	return DeviceMapper::get(database);
}

} // namespace config
} // namespace astro
