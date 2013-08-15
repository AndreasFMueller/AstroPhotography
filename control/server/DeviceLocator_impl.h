/*
 * DeviceLocator_impl.h -- DeviceLocator Corba interface
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceLocator_impl_h
#define _DeviceLocator_impl_h

#include <../idl/device.hh>
#include <AstroLoader.h>
#include <AstroDevice.h>

namespace Astro {

class DeviceLocator_impl : public POA_Astro::DeviceLocator {
	astro::device::DeviceLocatorPtr	_locator;
public:
	inline DeviceLocator_impl(astro::device::DeviceLocatorPtr locator)
		: _locator(locator) { }
	virtual ~DeviceLocator_impl() { }
	virtual char	*getName();
	virtual char	*getVersion();
	virtual ::Astro::DeviceLocator::DeviceNameList	*getDevicelist(
				::Astro::DeviceLocator::device_type devicetype);
};

} // namespace Astro

#endif /* _DeviceLocator_impl_h */
