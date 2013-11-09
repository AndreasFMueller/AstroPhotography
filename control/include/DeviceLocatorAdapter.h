/*
 * DeviceLocatorAdapter.h -- DeviceLocator adapter to unify device access
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DeviceLocatorAdapter_h
#define _DeviceLocatorAdapter_h

#include <module.hh>

namespace Astro {

template<typename Device_ptr>
class DeviceLocatorAdapter {
	DeviceLocator_var&	_devicelocator;
public:
	DeviceLocatorAdapter(DeviceLocator_var& devicelocator)
		: _devicelocator(devicelocator) { }
	Device_ptr	get(const char *name);
};

template<>
Camera_ptr	DeviceLocatorAdapter<Camera_ptr>::get(const char *name);

template<>
Ccd_ptr	DeviceLocatorAdapter<Ccd_ptr>::get(const char *name);

template<>
GuiderPort_ptr	DeviceLocatorAdapter<GuiderPort_ptr>::get(const char *name);

template<>
FilterWheel_ptr	DeviceLocatorAdapter<FilterWheel_ptr>::get(const char *name);

template<>
Cooler_ptr	DeviceLocatorAdapter<Cooler_ptr>::get(const char *name);

template<>
Focuser_ptr	DeviceLocatorAdapter<Focuser_ptr>::get(const char *name);

} // namespace Astro

#endif /* _DeviceLocatorAdapter_h */
