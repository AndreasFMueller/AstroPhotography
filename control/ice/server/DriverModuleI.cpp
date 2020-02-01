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

std::string	DriverModuleI::getName(const Ice::Current& current) {
	CallStatistics::count(current);
	return _module->getDescriptor()->name();
}

std::string	DriverModuleI::getVersion(const Ice::Current& current) {
	CallStatistics::count(current);
	return _module->getDescriptor()->version();
}

bool	DriverModuleI::hasLocator(const Ice::Current& current) {
	CallStatistics::count(current);
	return _module->getDescriptor()->hasDeviceLocator();
}

DeviceLocatorPrx	DriverModuleI::getDeviceLocator(
				const Ice::Current& current) {
	CallStatistics::count(current);
	std::string	modulename = _module->getDescriptor()->name();
	try {
		std::string	name = std::string("devicelocator/")
					+ modulename;
		return snowstar::createProxy<DeviceLocatorPrx>(name,
			current, false);
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot retrieve device locator for '%s': %s",
			modulename.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

} // namespace snowstar
