/*
 * DeviceNameUSB.h -- common device naming scheme for USB devices, currently
 *                    used by QHY and SX devices
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceNameUSB_h
#define _DeviceNameUSB_h

#include <AstroUSB.h>
#include <AstroDevice.h>

namespace astro {
namespace device {

/**
 * \brief Class to encapsulate all the naming logic
 */
class DeviceNameUSB {
	std::string	_modulename;
	unsigned short	_modulevendor;
public:
	const std::string& modulename() const { return _modulename; }
	unsigned short	modulevendor() const { return _modulevendor; }
protected:
	int	_busnumber;
	int	_deviceaddress;
public:
	int	busnumber() const { return _busnumber; }
	int	deviceaddress() const { return _deviceaddress; }
protected:
	std::string	iproduct;
	unsigned short	idvendor;
	unsigned short	idproduct;
	std::string	serial;

	void	parse(const std::string& name);
	std::string	unparse() const;
	bool	matches(const DeviceName& other, DeviceName::device_type type);
public:
	DeviceNameUSB(const std::string& modulename,
		unsigned short modulevendor, astro::usb::DevicePtr deviceptr);
	DeviceNameUSB(const std::string& modulename,
		unsigned short modulevendor,
		const astro::DeviceName& devicename);
	DeviceName	name(DeviceName::device_type type) const;
	DeviceName	name(DeviceName::device_type type,
				const std::string& path) const;
	DeviceName	name(DeviceName::device_type type,
				const std::vector<std::string>& path) const;
	std::string	stringname(DeviceName::device_type type) const;
	std::string	stringname(DeviceName::device_type type,
				const std::string& path) const;
	std::string	stringname(DeviceName::device_type type,
				const std::vector<std::string>& path) const;
	DeviceName	cameraname() const;
	DeviceName	ccdname() const;
	DeviceName	coolername() const;
	DeviceName	guiderportname() const;
	bool	isCamera(const DeviceName& other);
	bool	isCcd(const DeviceName& other);
	bool	isCooler(const DeviceName& other);
	bool	isGuiderport(const DeviceName& other);
};

} // namespace device
} // namespace astro

#endif /* _DeviceNameUSB_h */
