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

namespace astro {

/**
 * \brief Properties abstraction
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
 */
class DeviceName : public std::vector<std::string> {
public:
	typedef enum { AdaptiveOptics, Camera, Ccd, Cooler, Filterwheel,
		Focuser, Guiderport, Module } device_type;
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
	DeviceName(const DeviceName& other);
	// conversion to child device names of a different type
	DeviceName(const DeviceName& name, const device_type& type,
		const std::string& unitname);

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
	Device(const std::string& name);
	Device(const DeviceName& name);
	virtual ~Device();
	const DeviceName&	name() const { return _name; }
};

} // namespace device
} // namespace astro

#endif /* _AstroDevice_h */
