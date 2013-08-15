/*
 * DriverModule.cpp -- Driver module implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DriverModule.h"
#include "DeviceLocator.h"
#include <AstroDebug.h>

namespace Astro {

char	*DriverModule_impl::getName() {
	char	*result = strdup(_module->modulename().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning module name %s", result);
	return result;
}

Descriptor	*DriverModule_impl::getDescriptor() {
	Descriptor	*result = new Descriptor();
	result->name = strdup(_module->getDescriptor()->name().c_str());
	result->version = strdup(_module->getDescriptor()->version().c_str());
	return result;
}

DeviceLocator_ptr	DriverModule_impl::getDeviceLocator() {
	if (!_devicelocator) {
		_module->open();
		_devicelocator = _module->getDeviceLocator();
	}

	DeviceLocator_impl	*dl = new DeviceLocator_impl(_devicelocator);
	
	return dl->_this();
}

} // namespace astro
