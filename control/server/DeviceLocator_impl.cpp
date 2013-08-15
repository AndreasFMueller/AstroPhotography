/*
 * DeviceLocator_impl.cpp -- Driver module implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DeviceLocator_impl.h"
#include <AstroDebug.h>

namespace Astro {

char	*DeviceLocator_impl::getName() {
	return ::CORBA::string_dup(_locator->getName().c_str());
}

char	*DeviceLocator_impl::getVersion() {
	return ::CORBA::string_dup(_locator->getVersion().c_str());
}

::Astro::DeviceLocator::DeviceNameList	*DeviceLocator_impl::getDevicelist(::Astro::DeviceLocator::device_type devicetype) {
	astro::device::DeviceLocator::device_type	type;
	switch (devicetype) {
	case ::Astro::DeviceLocator::CAMERA:
		type = astro::device::DeviceLocator::CAMERA;
		break;
	case ::Astro::DeviceLocator::FOCUSER:
		type = astro::device::DeviceLocator::FOCUSER;
		break;
	case ::Astro::DeviceLocator::GUIDERPORT:
		type = astro::device::DeviceLocator::GUIDERPORT;
		break;
	}
	std::vector<std::string>	devices
		= _locator->getDevicelist(type);

	Astro::DeviceLocator::DeviceNameList	*result
		= new Astro::DeviceLocator::DeviceNameList();
	result->length(devices.size());
	std::vector<std::string>::const_iterator	i;
	int	j = 0;
	for (i = devices.begin(); i != devices.end(); i++) {
		(*result)[j++] = ::CORBA::string_dup(i->c_str());
	}
	return result;
}

} // namespace astro
