/*
 * DeviceLocator_impl.h -- DeviceLocator Corba interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceLocator_impl_h
#define _DeviceLocator_impl_h

#include <device.hh>
#include <AstroLoader.h>
#include <AstroDevice.h>

namespace Astro {

class DeviceLocator_impl : public POA_Astro::DeviceLocator {
	astro::device::DeviceLocatorPtr	_locator;
	std::map<std::string, astro::camera::CameraPtr>	cameramap;
	std::map<std::string, astro::camera::GuiderPortPtr>	guiderportmap;
	std::map<std::string, astro::camera::FilterWheelPtr>	filterwheelmap;
public:
	inline DeviceLocator_impl(astro::device::DeviceLocatorPtr locator)
		: _locator(locator) { }
	virtual ~DeviceLocator_impl() { }
	virtual char	*getName();
	virtual char	*getVersion();
	virtual ::Astro::DeviceLocator::DeviceNameList	*getDevicelist(
				::Astro::DeviceLocator::device_type devicetype);
	virtual Camera_ptr	getCamera(const char *name);
	virtual GuiderPort_ptr	getGuiderPort(const char *name);
	virtual FilterWheel_ptr	getFilterWheel(const char *name);
};

} // namespace Astro

#endif /* _DeviceLocator_impl_h */
