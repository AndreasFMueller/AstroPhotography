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

namespace astro {

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
	typedef enum { Camera, Ccd, Cooler, Filterwheel, Guiderport, Focuser }
		device_type;
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
	std::string	name() const;
public:
	DeviceName(const std::string& name);
	DeviceName(const std::string& modulename, const std::string& unitname);
	DeviceName(const device_type& type,
		const std::vector<std::string>& components);
	// conversion to device names of a different type
	DeviceName(const DeviceName& name, const device_type& type,
		const std::string& unitname);

	// comparison operators (for containers)
	bool	operator==(const DeviceName& other) const;
	bool	operator!=(const DeviceName& other) const;
	bool	operator<(const DeviceName& other) const;
	// cast to a string
	operator std::string() const;
};

std::ostream&	operator<<(std::ostream& out, const DeviceName& name);

namespace device {

/**
 * \brief Base class for all devices, handles device names
 *
 * Every device must have a DeviceName. The device name specifies the full
 * path to the device
 */
class Device {
protected:
	DeviceName	_name;
public:
	Device(const std::string& name) : _name(name) { }
	Device(const DeviceName& name) : _name(name) { }
	const DeviceName&	name() const { return _name; }
};

} // namespace device
} // namespace astro

#endif /* _AstroDevice_h */
