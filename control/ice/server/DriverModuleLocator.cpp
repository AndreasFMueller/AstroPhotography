/*
 * DriverModuleLocator.cpp -- locator for the driver module
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DriverModuleLocator.h>
#include <DriverModuleI.h>

namespace snowstar {

DriverModuleLocator::DriverModuleLocator(astro::module::Repository& repository)
	: _repository(repository) {
}

DriverModuleLocator::~DriverModuleLocator() {
}

Ice::ObjectPtr	DriverModuleLocator::locate(const Ice::Current& current,
			Ice::LocalObjectPtr& /* cookie */) {
	std::string	modulename = current.id.name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for module %s",
		modulename.c_str());

	// create a DriverModule for this module name
	return new DriverModuleI(_repository.getModule(modulename));
}

void	DriverModuleLocator::finished(const Ice::Current& /* current */,
		const Ice::ObjectPtr& /* servant */,
		const Ice::LocalObjectPtr& /* cookie */) {
}

void	DriverModuleLocator::deactivate(const std::string& /* category */) {
}

} // namespace snowstar
