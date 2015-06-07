/*
 * AstroDevice.h -- Device manager
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDevice_h
#define _AstroDevice_h

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <AstroCoordinates.h>
#include <memory>

namespace astro {

/**
 * \brief Properties abstraction
 *
 * Properties are configuration data associated with the hardware, and not
 * with the users. A camera attached to a multiuser system needs some 
 * configuration information that is the same for all users, like the
 * port it is connected to, or communication parameters. These would go
 * into the device properties file. The device properties file is typically
 * installed in system location, and not accessible for ordinary users.
 *
 * Probably the only clients of this class are the device driver modules,
 * so it is their responsibility to document the properties they would
 * like to read from a file. A possible example would be the serial port
 * that a Celestron mount would need. This information does not change over
 * time (USB parameters may be different each time the device is plugged in,
 * but there are usually methods to recognize the device and make it available
 * under a constant path name, see udev(7)). So the celestron driver would
 * read the device properties file typicall from a location like
 * /usr/local/etc/device.properties and find in it its configuration parameters
 * like the serial port device name, and the version of the protocol to
 * use. The celestron driver thus has to document the property names
 * I wants to read, e.g. celestron.mount.device and celestron.mount.version.
 *
 * Drivers also may use a naming scheme that allows for multiple devices
 * handled by the same driver. In the example of the celestron driver for
 * celestron mounts, the driver could use a device naming scheme like
 * mount:celestron/1, mount:celestron/2 etc., and then read the associated
 * configuration information from variables celestron.mount.1.device and
 * celestron.mount.2device respectively. The driver could also use the variable
 * without a device number as a default, which is overridden by the 
 * property that includes the device number in its name.
 *
 * Configuration information that depends on a particular user or project
 * is maintained in the configuration subsystem in AstroConfig.h, it uses
 * a database as its data store and some command line tools are offered 
 * to maintain this configuration information.
 */
class Properties {
public:
	typedef std::map<std::string, std::string>	propertymap_type;
private:
	propertymap_type	properties;
	void	setup(const std::string& name, const std::string& filename);
public:
	Properties(const std::string& devicename);
	virtual ~Properties();
	virtual bool	hasProperty(const std::string& name) const;
	virtual std::string	getProperty(const std::string& name) const;
	virtual std::string	getProperty(const std::string& name,
				const std::string& defaultvalue) const;
	virtual void	setProperty(const std::string& name,
				const std::string& value);
};

/**
 * \brief Name of a device
 *
 * In the extended device naming scheme, each fully qualified device  name
 * starts with a type designator, one of the strings "camera", "ccd",
 * "cooler", "filterwheel", "guiderport", "focuser", followed by a colon
 * and a sequence of path components separated by other slashes.
 *
 * Device names should encode physical paths that the driver can interpret.
 * It is okay if the path changes when the device is unplugged an replugged,
 * there is the astro::config::DeviceMapper class to map more user accessible
 * device names to these physical device names.
 */
class DeviceName : public std::vector<std::string> {
public:
	typedef enum { AdaptiveOptics, Camera, Ccd, Cooler, Filterwheel,
		Focuser, Guiderport, Module, Mount } device_type;
	static std::string	type2string(const device_type& type);
	static device_type	string2type(const std::string& name);
private:
	device_type	_type;
public:
	const device_type&	type() const { return _type; }
	std::string	typestring() const;
	void	type(const device_type& type) { _type = type; }
	void	typestring(const std::string& t);
	bool	hasType(const device_type& t) const;
public:
	const std::string&	modulename() const;
public:
	const std::string&	unitname() const;
	void	unitname(const std::string& u);
	std::string	name() const;
private:
	DeviceName() { }
public:
	DeviceName(const std::string& name);
	DeviceName(const std::string& modulename, const std::string& unitname);
	DeviceName(const device_type& type,
		const std::vector<std::string>& components);
	DeviceName(const device_type& type,
		const std::string& modulename, const std::string& unitname);
	DeviceName(const DeviceName& other);
	// conversion to child device names of a different type
	DeviceName(const DeviceName& name, const device_type& type,
		const std::string& unitname);
	DeviceName&	operator=(const DeviceName& other);

	// get the parent of a certain type
	DeviceName	parent(const device_type& devicetype) const;

	// comparison operators (for containers)
	bool	operator==(const DeviceName& other) const;
	bool	operator!=(const DeviceName& other) const;
	bool	operator<(const DeviceName& other) const;
	// cast to a string
	operator std::string() const;
	std::string	toString() const;
};

std::ostream&	operator<<(std::ostream& out, const DeviceName& name);

namespace device {

/**
 * \brief Base class for all devices, handles device names
 *
 * Every device must have a DeviceName. The device name specifies the full
 * path to the device
 */
class Device : public Properties {
protected:
	DeviceName	_name;
private:
	// make sure devices cannot be copied
	Device(const Device& other);
	Device&	operator=(const Device& other);
public:
	Device(const std::string& name, DeviceName::device_type type);
	Device(const DeviceName& name, DeviceName::device_type type);
	virtual ~Device();
	const DeviceName&	name() const { return _name; }
};

class Mount;
typedef std::shared_ptr<Mount>	MountPtr;

/**
 * \brief Base class for all Mounts
 *
 * A camera is mounted on a mount, together with the Telescope (which does
 * not have a class representing it). Mounts can return the current coordinates
 * the telescope is pointing to, and one can slew the telescope to a
 * given position using the Goto methods.
 */
class Mount : public Device {
public:
	typedef MountPtr	sharedptr;

	typedef enum state_type { IDLE, ALIGNED, TRACKING, GOTO } state_type;
	static std::string	state2string(state_type s);
	static state_type	string2state(const std::string& s);

	static DeviceName::device_type	devicetype;
	Mount(const std::string& name) : Device(name, DeviceName::Mount) { }
	Mount(const DeviceName& name) : Device(name, DeviceName::Mount) { }
	virtual ~Mount() { }

	// state
	virtual state_type	state() { return IDLE; }

	// position commands
	virtual RaDec	getRaDec();
	virtual AzmAlt	getAzmAlt();

	// goto commands
	virtual void	Goto(const RaDec& radec);
	virtual void	Goto(const AzmAlt& azmalt);
	virtual void	cancel();
};

} // namespace device
} // namespace astro

#endif /* _AstroDevice_h */
