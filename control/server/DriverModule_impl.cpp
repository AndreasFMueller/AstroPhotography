/*
 * DriverModule_impl.cpp -- Driver module implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DriverModule_impl.h"
#include "DeviceLocator_impl.h"
#include <AstroDebug.h>

namespace Astro {

/**
 * \brief Get the name of the module
 */
char	*DriverModule_impl::getName() {
	char	*result = strdup(_module->modulename().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning module name %s", result);
	return result;
}

/**
 * \brief Get the module descriptor
 */
Descriptor	*DriverModule_impl::getDescriptor() {
	Descriptor	*result = new Descriptor();
	result->name = strdup(_module->getDescriptor()->name().c_str());
	result->version = strdup(_module->getDescriptor()->version().c_str());
	result->hasDeviceLocator = _module->getDescriptor()->hasDeviceLocator();
	return result;
}

/**
 * \brief Get the DeviceLocator
 */
DeviceLocator_ptr	DriverModule_impl::getDeviceLocator() {
	if (!_module->getDescriptor()->hasDeviceLocator()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "module has no device locator");
		NotFound	notfound;
		notfound.cause = (const char *)"module has no device locator";
		throw notfound;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning device locator");
	DeviceLocator_impl	*dl
		= new DeviceLocator_impl(_module->getDeviceLocator());
	
	return dl->_this();
}

} // namespace astro
