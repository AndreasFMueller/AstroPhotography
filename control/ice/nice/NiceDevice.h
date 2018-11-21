/*
 * NiceDevice.h -- common information we keep for 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NiceDevice_h
#define _NiceDevice_h

#include <AstroDevice.h>
#include <Nice.h>

namespace astro {
namespace camera {
namespace nice {

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
