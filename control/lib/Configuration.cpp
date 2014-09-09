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
#include <ImageReposTable.h>
#include <AstroProject.h>

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
	virtual std::list<ConfigurationEntry>	globallist();

	// access to repositories
	virtual ImageRepo	repo(const std::string& name);
	virtual void	addrepo(const std::string& name,
				const std::string& directory);
	virtual void	removerepo(const std::string& name);
	virtual std::list<ImageRepoInfo>	listrepo();

	// devicemapper access
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

/**
 * \brief get a repository
 */
ImageRepo	ConfigurationBackend::repo(const std::string& name) {
	return ImageRepoTable(database).get(name);
}

/**
 * \brief add a repository
 */
void	ConfigurationBackend::addrepo(const std::string& name,
		const std::string& directory) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add image repo %s in directory %s",
		name.c_str(), directory.c_str());

	// first find out whether the repository already exists
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "test whether repo '%s' exists",
			name.c_str());
		repo(name);
		return;
	} catch (...) { }

	// prepare the entry for the database
	ImageRepoRecord	imagerepoinfo;
	imagerepoinfo.reponame = name;
	imagerepoinfo.database = directory + std::string("/.astro.db");
	imagerepoinfo.directory = directory;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using database name %s",
		imagerepoinfo.database.c_str());

	// create a new repository
	Database	db = DatabaseFactory::get(imagerepoinfo.database);
	ImageRepo	imagerepo(db, directory, true);

	// add the repository info to the database
	ImageRepoTable	repos(database);
	repos.add(imagerepoinfo);
}

/**
 * \brief delete a repository
 */
void	ConfigurationBackend::removerepo(const std::string& name) {
	ImageRepoTable(database).remove(name);
}

std::list<ImageRepoInfo>	ConfigurationBackend::listrepo() {
	ImageRepoTable	repos(database);
	std::list<ImageRepoInfo>	result;
	std::list<ImageRepoRecord>	repolist = repos.select("0 = 0");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d image repo records",
		repolist.size());
	std::list<ImageRepoRecord>::const_iterator	i;
	for (i = repolist.begin(); i != repolist.end(); i++) {
		ImageRepoInfo	info;
		info.reponame = i->reponame;
		info.database = i->database;
		info.directory = i->directory;
		result.push_back(info);
	}
	return result;
}


/**
 * \brief Get the device mapper
 */
DeviceMapperPtr	ConfigurationBackend::devicemapper() {
	return DeviceMapper::get(database);
}

/**
 * \brief 
 */
std::list<ConfigurationEntry>	ConfigurationBackend::globallist() {
	GlobalTable	globals(database);
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
