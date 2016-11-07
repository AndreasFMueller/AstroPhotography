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

#if 0
class Instrument;

/**
 * \brief Components of an instrument
 *
 * Components of an instrument are devices with different purposes. The
 * _type member indicates what purpose a component has. But there are
 * two ways to get a device name. When the device name is always the same,
 * as e.g. for the QSI cameras, then the component can refer to that
 * name directly. If, however, the device name may change, as for most
 * USB devices, then we need the device mapper, and the component should
 * only refer to the name of the map entry. The two different types of
 * components are implemented as derived classes of the InstrumentComponent
 * class.
 */
class InstrumentComponent {
	DeviceName::device_type	_type;
public:
	DeviceName::device_type	type() const { return _type; }
	std::string	type_name() const;

	typedef enum { direct, mapped, derived } component_t;
private:
	component_t	_component_type;
public:
	component_t	component_type() const { return _component_type; }
	std::string	component_typename() const;

	InstrumentComponent(DeviceName::device_type t, component_t c)
		: _type(t), _component_type(c) { }
	virtual	DeviceName	devicename() = 0;
	virtual	int	unit() = 0;
	virtual void	unit(int u) = 0;
	virtual std::string	name() const = 0;
	virtual void	name(const std::string& n) = 0;
	virtual std::string	servername() = 0;
	virtual std::string	toString();
};
typedef std::shared_ptr<InstrumentComponent>	InstrumentComponentPtr;

/**
 * \brief Mapped Instrument component
 *
 * This class represents instrument components that may change name
 * and thus need access to the DeviceMapper.
 */
class InstrumentComponentMapped : public InstrumentComponent {
	astro::persistence::Database	_database;
	std::string	_name;
public:
	InstrumentComponentMapped(DeviceName::device_type t,
		astro::persistence::Database database, const std::string& name)
		: InstrumentComponent(t, InstrumentComponent::mapped),
		  _database(database), _name(name) { }
	virtual DeviceName	devicename();
	virtual int	unit();
	virtual void	unit(int u);
	virtual std::string	name() const;
	virtual void	name(const std::string& n);
	virtual std::string	servername();
};

/**
 * \brief  Direct instrument component
 *
 * This class represents instrument components that have a permanent
 * device name assignment, i.e. that don't need the DeviceMapper to resolve
 * the device name.
 */
class InstrumentComponentDirect : public InstrumentComponent {
	DeviceName	_devicename;
	int	_unit;
	std::string	_servername;
public:
	InstrumentComponentDirect(DeviceName::device_type t,
		const DeviceName& devicename, int unit,
		const std::string& servername)
		: InstrumentComponent(t, InstrumentComponent::direct),
		  _devicename(devicename), _unit(unit),
		  _servername(servername) {
	}
	virtual DeviceName	devicename() { return _devicename; }
	virtual int	unit() { return _unit; }
	virtual void	unit(int u) { _unit = u; }
	virtual std::string	name() const { return _devicename.toString(); }
	virtual void	name(const std::string& n) {
		_devicename = DeviceName(n);
	}
	virtual std::string	servername() { return _servername; }
	virtual void	servername(const std::string& s) { _servername = s; }
};

class Instrument;
typedef std::shared_ptr<Instrument>	InstrumentPtr;

/**
 * \brief Derived instrument component
 *
 * This class represents an instrument component that is also a component
 * of some other device. E.g. the Guider port is often a component of
 * a camera, and similarly for filterwheels, 
 */
class InstrumentComponentDerived : public InstrumentComponent {
	Instrument&		_instrument;
	DeviceName::device_type	_derivedfrom;
	int			_unit;
public:
	InstrumentComponentDerived(DeviceName::device_type t,
		Instrument& instrument, DeviceName::device_type derivedfrom,
		int unit)
		: InstrumentComponent(t, InstrumentComponent::derived),
		  _instrument(instrument), _derivedfrom(derivedfrom),
		  _unit(unit) {
	}
	virtual DeviceName	devicename();
	virtual int	unit() { return _unit; }
	virtual void	unit(int u) { _unit = u; }
	virtual std::string	name() const;
	virtual void	name(const std::string& n);
	virtual std::string	servername();
	DeviceName::device_type	derivedfrom() const { return _derivedfrom; }
};


/**
 * \brief Instrument abstraction
 *
 * Instruments are collections of devices that need to be controlled in
 * unison for successful execution of a project. It will almost always
 * contain a camera device, but sometimes it will also include coolers,
 * filterwheels, adaptive optics units, or the mount, even if it is just
 * used to retrieve metadata.
 *
 * Projects use Instruments, as they can find all they need within in
 * Instrument object. The Instrument object has two different ways how
 * it can access a component. Either the component device name is fixed
 * over time, or the device mapper should be used to find the device name
 * that is currently valid.
 */
class Instrument {
private:
	astro::persistence::Database	_database;
	std::string	_name;
public:
	const std::string& name() const { return _name; }

	typedef	std::map<DeviceName::device_type, InstrumentComponentPtr>	component_map;
private:
	component_map	components;
public:
	Instrument(astro::persistence::Database database,
		const std::string& name);
	bool	has(const DeviceName::device_type type) const;
	bool	isLocal(const DeviceName::device_type type) const;

	InstrumentComponentPtr	component(DeviceName::device_type type) const;

	InstrumentComponent::component_t	component_type(DeviceName::device_type type) const;

	DeviceName	devicename(DeviceName::device_type type);
	std::string	name(DeviceName::device_type type);
	int	unit(DeviceName::device_type type);
	std::string	servername(DeviceName::device_type type);

	void	add(InstrumentComponentPtr component);
	void	remove(DeviceName::device_type type);

	int	ncomponents() const { return components.size(); }
	std::string	toString() const;

	std::list<DeviceName::device_type>	component_types() const;

	// retrieve devices from an instrument
	astro::camera::AdaptiveOpticsPtr	adaptiveoptics();
	astro::camera::CameraPtr		camera();
	astro::camera::CcdPtr			ccd();
	astro::camera::CoolerPtr		cooler();
	astro::camera::FilterWheelPtr		filterwheel();
	astro::camera::FocuserPtr		focuser();
	astro::camera::GuidePortPtr		guideport();
	astro::device::MountPtr			mount();
};
#endif

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

	// access to the raw database
	virtual persistence::Database	database() = 0;
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

#if 0
class InstrumentConfiguration;
typedef std::shared_ptr<InstrumentConfiguration>	InstrumentConfigurationPtr;
class InstrumentConfiguration {
public:
static InstrumentConfigurationPtr	get();
static InstrumentConfigurationPtr	get(ConfigurationPtr config);

	// instrument access
	virtual InstrumentPtr	instrument(const std::string& name) = 0;
	virtual void	addInstrument(InstrumentPtr instrument) = 0;
	virtual void	removeInstrument(const std::string& name) = 0;
	virtual std::list<InstrumentPtr>	listinstruments() = 0;
};
#endif

} // namespace config
} // namespace astro

#endif /* _AstroConfig_h */
