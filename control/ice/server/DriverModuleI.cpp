/*
 * DriverModuleI.cpp -- implementation of servant for driver modules
 *
 * (c) 2014 Prof Dr Andreas Muelller, Hochschule Rapperswil
 */
#include <DriverModuleI.h>
#include <ProxyCreator.h>

namespace snowstar {

DriverModuleI::DriverModuleI(astro::module::ModulePtr module)
	: _module(module) {
}

DriverModuleI::~DriverModuleI() {
}

std::string	DriverModuleI::getName(const Ice::Current& /* current */) {
	return _module->getDescriptor()->name();
}

std::string	DriverModuleI::getVersion(const Ice::Current& /* current */) {
	return _module->getDescriptor()->version();
}

bool	DriverModuleI::hasLocator(const Ice::Current& /* current */) {
	return _module->getDescriptor()->hasDeviceLocator();
}

DeviceLocatorPrx	DriverModuleI::getDeviceLocator(const Ice::Current& current) {
	return snowstar::createProxy<DeviceLocatorPrx>(std::string("devicelocator/") + _module->getDescriptor()->name(), current);
}

} // namespace snowstar
