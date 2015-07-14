/*
 * NiceDevice.h -- common information we keep for 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceDevice_h
#define _NiceDevice_h

#include <AstroDevice.h>

namespace astro {
namespace camera {
namespace nice {

/**
 * \brief Class to strip network information from a nice device name
 */
class DeviceNicer {
	std::string	_servicename;
public:
	DeviceNicer(const std::string& servicename);
	DeviceName	operator()(const DeviceName& devicename);
	std::string	operator()(const std::string& name);
	std::vector<std::string>	operator()(
		const std::vector<std::string>& names);
};

/**
 * \brief Class to convert net names to local names
 *
 * This class converts a device name suitable for the 'nice' driver 
 * to a local name. This means removing the service name component
 */
class DeviceDenicer {
	std::shared_ptr<DeviceName>	_devicename;
	std::string	_service;
	void	setup(const DeviceName& original);
public:
	DeviceDenicer(const std::string& device);
	DeviceDenicer(const DeviceName& original);
	std::string	service() const;
	DeviceName	devicename() const;
};


/**
 * \brief Base class for Nice devices
 *
 * The base class store information related to the network connection of
 * the device
 */
class NiceDevice {
	std::string	_service;
	std::shared_ptr<DeviceName>	_localname;
public:
	const std::string&	service() const { return _service; }
	const DeviceName&	localname() const { return *_localname; }
public:
	NiceDevice(const DeviceName& devicename);
	virtual ~NiceDevice();
	DeviceName	nice(const DeviceName& name);
};

} // namespace nice
} // namespace camera
} // namespace astro

#endif /* _NiceDevice_h */
