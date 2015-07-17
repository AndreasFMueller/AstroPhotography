/*
 * Nice.h -- classes to convert nice urls into local urls and back
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Nice_h
#define _Nice_h

#include <AstroDevice.h>

namespace astro {
namespace device {
namespace nice {

/**
 * \brief Class to strip network information from a nice device name
 *
 * This class converts local device names of the form camera:module/path
 * to a device name for the nice driver: camera:nice/service/module/path,
 * where <service> is the name of the zeroconf service name under which
 * a device can be found.
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
 * to a local name. This means removing the nice module name and the
 * service name component, i.e. it converts a nice name of the form
 * camera:nice/service/module/path to a local name of the form
 * camera:module/path
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

} // namespace nice
} // namespace device
} // namespace astro

#endif /* _Nice_h */
