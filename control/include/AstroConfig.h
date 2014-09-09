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
#include <memory>

namespace astro {
namespace config {

/**
 * \brief Device mapping entry
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
static DeviceMapperPtr	get(astro::persistence::Database database);
};

typedef std::shared_ptr<Configuration>	ConfigurationPtr;

class ConfigurationEntry {
public:
	std::string	section;
	std::string	name;
	std::string	value;
};

/**
 * \brief Configuration repository class
 *
 * All configuration information can be accessed through this interface
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

	// global configuration variables
	virtual std::string	global(const std::string& section,
				const std::string& name) = 0;
	virtual std::string	global(const std::string& section,
				const std::string& name,
				const std::string& def) = 0;
	virtual void	setglobal(const std::string& section,
			const std::string& name, const std::string& value) = 0;
	virtual void	removeglobal(const std::string& section,
				const std::string& name) = 0;
	virtual std::list<ConfigurationEntry>	globallist() = 0;

	// image repository access
	virtual astro::project::ImageRepo	repo(const std::string& name) = 0;
	virtual void	addrepo(const std::string& name,
				const std::string& directory) = 0;
	virtual void	removerepo(const std::string& name) = 0;
	virtual std::list<project::ImageRepoInfo>	listrepo() = 0;

	// device mapper stuff
	virtual DeviceMapperPtr	devicemapper() = 0;
};

} // namespace config
} // namespace astro

#endif /* _AstroConfig_h */
