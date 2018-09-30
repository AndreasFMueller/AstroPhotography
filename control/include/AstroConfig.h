/*
 * AstroConfig.h -- classes for configuration management of the AP application
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroConfig_h
#define _AstroConfig_h

#include <AstroPersistence.h>
#include <AstroDevice.h>
#include <AstroProject.h>
#include <AstroLoader.h>
#include <memory>

namespace astro {

/**
 * \brief Namespace for the configuration subsystem
 *
 * All classes helping to maintain user accessible configuration information
 * are in this namespace.
 */
namespace config {

/**
 * \brief Device mapping entry
 *
 * The class astro::config::DeviceMapper performs mapping from physical 
 * device names to user acceptable device names, using a map of 
 * astro::config::DeviceMap entries. Such an entry contains information
 * about the physical path (_devicename and _unitid), and the location
 * where the device is installed (_servername). The unchanging name of
 * a device is its _name.
 */
class DeviceMap {
	std::string	_name;
	DeviceName	_devicename;
	int		_unitid;
	std::string	_servername;
	std::string	_description;
public:
	DeviceMap(const DeviceName& devicename) : _devicename(devicename),
		_unitid(0) {
	}

	const std::string&	name() const { return _name; }
	void	name(const std::string& n) { _name = n; }

	int	unitid() const { return _unitid; }
	void	unitid(int i) { _unitid = i; }

	const std::string&	servername() const { return _servername; }
	void	servername(const std::string& s) { _servername = s; }

	const std::string&	description() const { return _description; }
	void	description(const std::string& d) { _description = d; }

	const DeviceName&	devicename() const { return _devicename; }
	void	devicename(const DeviceName& d) { _devicename = d; }
};

class DeviceMapper;
typedef std::shared_ptr<DeviceMapper>	DeviceMapperPtr;

class Configuration;

/**
 * \brief  Device Mapper class
 *
 * USB devices in particular change physical connection parameters when 
 * they are plugged in. To some degree, udev(7) can mitigate that problem
 * and ensure a stable device path name is used. However, this typically
 * does work for USB only to a limited degree, applications will still have
 * to scan the bus for the device they are looking for. And they will need
 * some information to map the probably changing physical device path
 * (usually encoded in a device name url (see astro::DeviceName) to a
 * short and mostly constant identifier that can also be used to document
 * the origin of exposures. This mapping is performed by the DeviceMapper
 * class.
 *
 * The device map is maintained with the devicemapper command line tool.
 * The idea is that before using a device with any other tool or script,
 * the devicemapper command is used to establish the mapping of the devices
 * present to the easy to use short names. A program using the device is 
 * then given the short name, and looks up the physical device name using
 * the DeviceMapper. This allows scripts to be written with an unchanging
 * name for the device, which makes them simpler and more robust.
 */
class DeviceMapper {
	DeviceMap	select(const std::string& condition);
public:
	virtual DeviceMap	find(const std::string& name) = 0;
	virtual DeviceMap	find(const DeviceName& devicename, int unitid,
					const std::string& servername) = 0;
	virtual void	add(const DeviceMap& devicemap) = 0;
	virtual void	update(const std::string& name,
				const DeviceMap& devicemap) = 0;
	virtual void	update(const DeviceName& devicename, int unitid,
				const std::string& servername,
				const DeviceMap& devicemap) = 0;
	virtual void	remove(const std::string& name) = 0;
	virtual void	remove(const DeviceName& devicename, int unitid,
				const std::string& servername) = 0;
	virtual std::list<DeviceMap>	select() = 0;
static DeviceMapperPtr	get(astro::persistence::Database database);
};

typedef std::shared_ptr<Configuration>	ConfigurationPtr;

/**
 * \brief Key class for configuration entries
 */
class ConfigurationKey {
public:
	std::string	domain;
	std::string	section;
	std::string	name;
	ConfigurationKey();
	ConfigurationKey(const std::string& _domain,
		const std::string& _section, const std::string& _name);
	ConfigurationKey(const ConfigurationKey& other);
	ConfigurationKey&	operator=(const ConfigurationKey& other);
	bool	operator==(const ConfigurationKey& other) const;
	bool	operator<(const ConfigurationKey& other) const;
	std::string	condition() const;
	std::string	toString() const;
};

/**
 * \brief Configuration database entry
 *
 * This is a holder class for configuration data.
 */
class ConfigurationEntry : public ConfigurationKey {
public:
	std::string	value;
	ConfigurationEntry();
	ConfigurationEntry(const std::string& _domain,
		const std::string& _section,
		const std::string& _name, const std::string& _value);
	ConfigurationEntry(const ConfigurationKey& key,
		const std::string& _value);
	bool	operator==(const ConfigurationEntry& other) const;
	bool	operator<(const ConfigurationEntry& other) const;
};

/**
 * \brief exception class to use when entries are not found
 */
class NoSuchEntry : public std::runtime_error {
public:
	NoSuchEntry(const std::string& domain, const std::string& section,
		const std::string& name);
	NoSuchEntry(const std::string& cause);
	NoSuchEntry();
};

/**
 * \brief Configuration repository class
 *
 * All configuration information can be accessed through this interface.
 * Currently there are for types of configuration information stored in 
 * a configuration object. Such configuration objects are backed by
 * a sqlite3 database file, the static methods of the class are factory
 * methods to produce astro::config::Configuration objects from database
 * files.
 *
 * Since it would be very cumbersome to hand around Configuration objects,
 * there are static methods to set the default configuration. The default
 * configuration is usually a database file in the users home directory.
 * Then applications can use the Configuration::get() method to retrieve
 * the default configuration. The backend will keep a cache of configurations
 * so ony one copy of each configuration will be instantiated.
 *
 * First there is global configuration information. These are configuration
 * attribute value pairs organized in sections, similar to INI-files.
 *
 * Images are stored in image repositories, which simplify access to image
 * meta data. For ease of use, image repositories should have short names,
 * and in a shared environment, they should even be global, i.e. on each
 * system, one should be able to use the same repository name, even if the
 * the images are available through different paths. This potentially system
 * specific information about the location of image repositories is also
 * maintained in the configuration object.
 *
 * Images are grouped using projects. A project contains some additional
 * information like the object to be imaged, and the time when it started.
 * In the future, more information like a complete plan about the number
 * of light images to be taken with each filter and the exposure times,
 * and even the processing of those images will go into the configuration.
 * Projects are also stored in the configuration database file. The most
 * important parameter at the moment is the name of the repository that
 * should be used to store images produced by programs that work for this
 * project.
 *
 * Tools that tie into this configuration system should therefore only use
 * names for devicese that are resolved by the device mapper, they should
 * always specify a project, and they should always use the image repository
 * specified in the project to store their images.
 *
 * This base class does not reveal how the data is stored, all methods
 * are pure virtual. The derived class astro::config::ConfigurationBackend
 * implements all this methods. The static methods return instances of
 * astro::config::ConfigurationBackend wrapped in a smart pointer named
 * ConfigurationPtr. The methods of the Configuration class are only
 * documented in the backend class.
 */
class Configuration {
private: // prevent copying of configuration
	Configuration(const Configuration& other);
	Configuration&	operator=(const Configuration& other);
public:
	Configuration() { }
	virtual ~Configuration() { }
	// factory methods for the configuration
static ConfigurationPtr	get();
static ConfigurationPtr	get(const std::string& filename);
static std::string	get_default();
static void	set_default(const std::string& filename);

	// all configuration variables
	virtual bool	has(const ConfigurationKey& key) = 0;
	virtual bool	has(const std::string& domain,
				const std::string& section,
				const std::string& name) = 0;
	virtual std::string	get(const ConfigurationKey& key) = 0;
	virtual std::string	get(const std::string& domain,
					const std::string& section,
					const std::string& name) = 0;
	virtual std::string	get(const std::string& domain,
					const std::string& section,
					const std::string& name,
					const std::string& def) = 0;
	virtual void	set(const std::string& domain,
				const std::string& section,
				const std::string& name,
				const std::string& value) = 0;
	virtual void	set(const ConfigurationKey& key,
				const std::string& value) = 0;
	virtual void	remove(const std::string& domain,
				const std::string& section,
				const std::string& name) = 0;
	virtual void	remove(const ConfigurationKey& key) = 0;
	virtual std::list<ConfigurationEntry>	list() = 0;
	virtual std::list<ConfigurationEntry>	list(const std::string& domain) = 0;
	virtual std::list<ConfigurationEntry>	list(const std::string& domain,
		const std::string& section) = 0;

	// simplified accessors
	virtual void	setMediaPath(const std::string& path) = 0;
	virtual std::string	getMediaPath() = 0;

	// access to the raw database
	virtual persistence::Database	database() = 0;

	virtual persistence::Database	systemdatabase();
	virtual persistence::Database	mediadatabase() = 0;

	// registration of keys
	static void	registerkey(const ConfigurationKey& key,
				const std::string& description);
	static void	registerkey(const std::string& domain,
				const std::string& section,
				const std::string& name,
				const std::string& description) {
		registerkey(ConfigurationKey(domain, section, name),
			description);
	}
	static std::string	describe(const ConfigurationKey& key);
	static std::list<ConfigurationKey>	listRegistered();
	static void	showkeys(std::ostream& out,
				bool showdescriptions = false);
};

class ConfigurationRegister : public ConfigurationKey {
public:
	ConfigurationRegister(const std::string& domain,
		const std::string& section, const std::string& name,
		const std::string& description);
};

class ImageRepoConfiguration;
typedef std::shared_ptr<ImageRepoConfiguration>	ImageRepoConfigurationPtr;
class ImageRepoConfiguration {
public:	
static ImageRepoConfigurationPtr	get();
static ImageRepoConfigurationPtr	get(ConfigurationPtr config);
	// image repository access
	virtual bool	exists(const std::string& name) = 0;
	virtual astro::project::ImageRepoPtr	repo(const std::string& name) = 0;
	virtual void	addrepo(const std::string& name,
				const std::string& directory) = 0;
	virtual void	removerepo(const std::string& name,
				bool removecontents = false) = 0;
	virtual std::list<project::ImageRepoInfo>	listrepo(bool hidden_only) = 0;
	virtual bool	hidden(const std::string& name) = 0;
	virtual void	setHidden(const std::string& name, bool hidden) = 0;
};

class ProjectConfiguration;
typedef std::shared_ptr<ProjectConfiguration>	ProjectConfigurationPtr;
class ProjectConfiguration {
public:
static ProjectConfigurationPtr	get();
static ProjectConfigurationPtr	get(ConfigurationPtr config);

	// project definition
	virtual project::Project	project(const std::string& name) = 0;
	virtual void	addproject(const project::Project& project) = 0;
	virtual void	removeproject(const std::string& name) = 0;
	virtual std::list<project::Project>	listprojects() = 0;

	virtual project::PartPtr	part(const std::string& projectname,
						long partno) = 0;
	virtual void	addpart(const std::string& projectname,
					const project::Part& part) = 0;
	virtual void	removepart(const std::string& projectname,
					long partno) = 0;
	virtual std::list<project::PartPtr>	listparts(
					const std::string& projectname) = 0;
	virtual void	parttask(const std::string& projectname, long partno,
				int taskid) = 0;
	virtual void	partrepo(const std::string& projectname, long partno,
				int repoid) = 0;
};

class DeviceMapperConfiguration;
typedef std::shared_ptr<DeviceMapperConfiguration>	DeviceMapperConfigurationPtr;
class DeviceMapperConfiguration {
public:
static DeviceMapperConfigurationPtr	get();
static DeviceMapperConfigurationPtr	get(ConfigurationPtr config);

	// device mapper stuff
	virtual DeviceMapperPtr	devicemapper() = 0;
};

} // namespace config
} // namespace astro

#endif /* _AstroConfig_h */
