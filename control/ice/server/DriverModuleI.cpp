/*
 * DriverModuleI.cpp -- DriverModule 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DriverModuleI.h>

namespace snowstar {

std::string	DriverModuleI::getName(const ::Ice::Current& current) {
	return _module->modulename();
}

Descriptor	DriverModuleI::getDescriptor(const ::Ice::Current& current) {
	Descriptor	descriptor;
	descriptor.name = _module->getDescriptor()->name();
	descriptor.version = _module->getDescriptor()->version();
	descriptor.hasDeviceLocator
		= _module->getDescriptor()->hasDeviceLocator();
	return descriptor;
}

DeviceLocatorPtr	DriverModuleI::getDeviceLocator(
				const ::Ice::Current& current) {
	return NULL;
}

} // namespace snowstar


