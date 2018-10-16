/*
 * DeviceLocatorLocator.cpp -- locator for DeviceLocator servants
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceLocatorLocator.h>
#include <DeviceLocatorI.h>

namespace snowstar {

DeviceLocatorLocator::DeviceLocatorLocator(astro::module::ModuleRepositoryPtr repository) : _repository(repository) {
}

DeviceLocatorLocator::~DeviceLocatorLocator() {
}

Ice::ObjectPtr	DeviceLocatorLocator::locate(const Ice::Current& current,
			Ice::LocalObjectPtr& /* cookie */) {
	std::string	modulename = current.id.name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for locator in module '%s'",
		modulename.c_str());
	try {
		astro::module::ModulePtr	module
			= _repository->getModule(modulename);
		return new DeviceLocatorI(module->getDeviceLocator());
	} catch (const std::exception& x) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf(
			"cannot get DeviceLocator '%s': %s",
			modulename.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
}

void	DeviceLocatorLocator::finished(const Ice::Current& /* current */,
				const Ice::ObjectPtr& /* servant */,
				const Ice::LocalObjectPtr& /* cookie */) {
}

void	DeviceLocatorLocator::deactivate(const std::string& /* category */) {
}


} // namespace snowstar
