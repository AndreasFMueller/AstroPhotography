/*
 * DeviceLocatorAdapter.cpp --
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceLocatorAdapter.h>

namespace Astro {

template<>
Camera_ptr	DeviceLocatorAdapter<Camera_ptr>::get(const char *name) {
	return _devicelocator->getCamera(name);
}

template<>
Ccd_ptr	DeviceLocatorAdapter<Ccd_ptr>::get(const char *name) {
	return _devicelocator->getCcd(name);
}

template<>
GuiderPort_ptr	DeviceLocatorAdapter<GuiderPort_ptr>::get(const char *name) {
	return _devicelocator->getGuiderPort(name);
}

template<>
FilterWheel_ptr	DeviceLocatorAdapter<FilterWheel_ptr>::get(const char *name) {
	return _devicelocator->getFilterWheel(name);
}

template<>
Cooler_ptr	DeviceLocatorAdapter<Cooler_ptr>::get(const char *name) {
	return _devicelocator->getCooler(name);
}

template<>
Focuser_ptr	DeviceLocatorAdapter<Focuser_ptr>::get(const char *name) {
	return _devicelocator->getFocuser(name);
}


} // namespace Astro
