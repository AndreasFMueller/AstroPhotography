/*
 * NetLocator.h -- Device locator class for Corba
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetLocator_h
#define _NetLocator_h

#include "../../idl/device.hh"

namespace astro {
namespace camera {
namespace net {

class NetLocator : public astro::device::DeviceLocator {
	Astro::Modules_var	modules;
	std::string	modulename(const std::string& netname);
	std::string	devicename(const std::string& netname);
public:
	NetLocator();
	virtual ~NetLocator();

	virtual std::string	getName();
	virtual std::string	getVersion();
	virtual std::vector<std::string>	getDeviceList(
		DeviceLocator::device_type device = DevcieLocator::CAMERA);
protected:
	virtual CameraPtr	getCamera0(const std::string& name);
	virtual FilterWheelPtr	getFilterWheel0(const std::string& name);
	virtual GuiderPortPtr	getGuiderPort0(const std::string& name);
	virtual CoolerPtr	getCooler0(const std::string& name);
};

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetLocator_h */
