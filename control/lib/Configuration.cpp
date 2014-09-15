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
#include <ProjectTable.h>
#include <InstrumentTables.h>

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

	// project definition
	virtual project::Project	project(const std::string& name);
	virtual void	addproject(const project::Project& project);
	virtual void	removeproject(const std::string& name);
	virtual std::list<project::Project>	listprojects();

	// instrument access
	virtual InstrumentPtr	instrument(const std::string& name);
	virtual void    addInstrument(InstrumentPtr instrument);
	virtual void    removeInstrument(const std::string& name);
	virtual std::list<InstrumentPtr>   listinstruments();

	// devicemapper access
	virtual DeviceMapperPtr	devicemapper();

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
// repository access
//////////////////////////////////////////////////////////////////////
/**
 * \brief get a repository
 */
ImageRepo	ConfigurationBackend::repo(const std::string& name) {
	return ImageRepoTable(_database).get(name);
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
	ImageRepo	imagerepo(name, db, directory, false);

	// add the repository info to the database
	ImageRepoTable	repos(_database);
	repos.add(imagerepoinfo);
}

/**
 * \brief delete a repository
 */
void	ConfigurationBackend::removerepo(const std::string& name) {
	ImageRepoTable(_database).remove(name);
}

std::list<ImageRepoInfo>	ConfigurationBackend::listrepo() {
	ImageRepoTable	repos(_database);
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

//////////////////////////////////////////////////////////////////////
// project access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Get a project from the configuration
 */
Project	ConfigurationBackend::project(const std::string& name) {
	ProjectTable	projects(_database);
	ProjectRecord	record = projects.get(name);
	Project	project;
	project.name(record.name);
	project.description(record.description);
	project.object(record.object);
	project.repository(record.repository);
	project.started(record.started);
	return project;
}

/**
 * \brief add a project to the configuration
 */
void	ConfigurationBackend::addproject(const Project& project) {
	ProjectTable	projects(_database);
	ProjectRecord	record;
	record.name = project.name();
	record.description = project.description();
	record.object = project.object();
	record.started = project.started();
	record.repository = project.repository();
	projects.add(record);
}

/**
 * \brief Remove a project
 */
void	ConfigurationBackend::removeproject(const std::string& name) {
	ProjectTable	projects(_database);
	projects.remove(name);
}

/**
 * \brief Get a list of projects defined in this configuration
 */
std::list<Project>	ConfigurationBackend::listprojects() {
	std::list<Project>	result;
	ProjectTable	projects(_database);
	std::list<ProjectRecord>	records = projects.select("0 = 0");
	std::list<ProjectRecord>::const_iterator	pi;
	for (pi = records.begin(); pi != records.end(); pi++) {
		Project	project;
		project.name(pi->name);
		project.description(pi->description);
		project.object(pi->object);
		project.started(pi->started);
		project.repository(pi->repository);
		result.push_back(project);
	}
	return result;
}

//////////////////////////////////////////////////////////////////////
// device mapper access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Get the device mapper
 */
DeviceMapperPtr	ConfigurationBackend::devicemapper() {
	return DeviceMapper::get(_database);
}

//////////////////////////////////////////////////////////////////////
// Instrument access
//////////////////////////////////////////////////////////////////////
/**
 * \brief Retrieve an Instrument from the database
 */
InstrumentPtr	ConfigurationBackend::instrument(const std::string& name) {
	// find the id
	InstrumentTable	instruments(_database);
	int	instrumentid = instruments.id(name);

	// retrieve the instrument record
	InstrumentRecord	instrumentrecord
					= instruments.byid(instrumentid);
	InstrumentPtr	instrument(new Instrument(_database,
				instrumentrecord.name));

	// retrieve all the matching metadata
	InstrumentComponentTable	components(_database);
	std::string	condition = stringprintf("instrument = %d",
						instrumentid);
	auto l = components.select(condition);
	for (auto ptr = l.begin(); ptr != l.end(); ptr++) {
		// find the type from the string version of the type
		DeviceName::device_type	type
			= InstrumentComponentTableAdapter::type(ptr->type);
		InstrumentComponent::component_t	ctype
			= InstrumentComponentTableAdapter::component_type(
				ptr->componenttype);

		// construct suitable InstrumentComponent objects depending
		// on the mapped field of the component record
		InstrumentComponentPtr	iptr;
		switch (ctype) {
		case InstrumentComponent::mapped:
			// in the case of mapped devices, teh device name is
			// not an actuald evie name, but rather the name of
			// the map entry
			iptr = InstrumentComponentPtr(
				new InstrumentComponentMapped(type, _database,
					ptr->devicename));
			break;
		case InstrumentComponent::direct:
			// for direct components, matters are simplest, so
			// all fields have the meaning the name suggest
			iptr = InstrumentComponentPtr(
				new InstrumentComponentDirect(type,
					DeviceName(ptr->devicename),
					ptr->unit));
			break;
		case InstrumentComponent::derived:
			// in this case, the devicename is really the component
			// type from which the component should be derived
			iptr = InstrumentComponentPtr(
				new InstrumentComponentDerived(type, instrument,
					InstrumentComponentTableAdapter::type(
						ptr->devicename),
					ptr->unit));
			break;
		}

		// add the new component
		instrument->add(iptr);
	}

	// return the instrument
	return instrument;
}

/**
 * \brief Convert a InstrumentComponentPtr to an InstrumentComponentRecord
 */
static InstrumentComponentRecord	componentrecord(long instrumentid,
					InstrumentComponentPtr& component) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding component %s",
		component->name().c_str());
	InstrumentComponentRecord	componentrecord(-1, instrumentid);

	// assign the various members
	componentrecord.unit = component->unit();
	componentrecord.componenttype
		= InstrumentComponentTableAdapter::component_type(
			component->component_type());
	componentrecord.type
		= InstrumentComponentTableAdapter::type(
			component->type());
	componentrecord.devicename = component->name();

	// that's it, return the record
	return componentrecord;
}

/**
 * \brief Add an instrument to the database
 */
void	ConfigurationBackend::addInstrument(InstrumentPtr instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add instrument '%s' to the database",
		instrument->name().c_str());

	// open a transaction bracket
	_database->begin("addinstrument");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transaction opened");

	try {
		// create an instrument entry
		InstrumentTable	instruments(_database);
		InstrumentRecord	instrumentrecord;
		instrumentrecord.name = instrument->name();
		long	instrumentid = instruments.add(instrumentrecord);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "id of new instrument: %d",
			instrumentid);

		// for each component type, create an entry if the type
		// is present
		InstrumentComponentTable	components(_database);
		std::list<DeviceName::device_type>	types
			= instrument->component_types();
		for (auto ptr = types.begin(); ptr != types.end(); ptr++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "component of type %d",
				*ptr);
			DeviceName::device_type	devtype = *ptr;
			InstrumentComponentPtr	cptr
				= instrument->component(devtype);
			components.add(componentrecord(instrumentid, cptr));
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "entry complete");

		// commit the additions
		_database->commit("addinstrument");
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to add '%s': %s",
			instrument->name().c_str(), x.what());
		_database->rollback("addinstrument");
		throw;
	}
}


/**
 * \brief Remove an instrument from the tables
 */
void	ConfigurationBackend::removeInstrument(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove instrument named '%s'",
		name.c_str());
	InstrumentTable	instruments(_database);
	long	instrumentid = instruments.id(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete instrument id = %ld",
		instrumentid);
	instruments.remove(instrumentid);
}

/**
 * \brief List all instruments in the database
 */
std::list<InstrumentPtr>	ConfigurationBackend::listinstruments() {
	std::list<InstrumentPtr>	result;
	InstrumentTable	instruments(_database);
	std::list<InstrumentRecord>	records = instruments.select("0 = 0");
	for (auto ptr = records.begin(); ptr != records.end(); ptr++) {
		result.push_back(instrument(ptr->name));
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
