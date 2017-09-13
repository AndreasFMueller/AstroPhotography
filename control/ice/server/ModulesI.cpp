/*
 * ModulesI.cpp -- implementation of the Modules servant
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ModulesI.h>
#include <ProxyCreator.h>
#include <device.h>

namespace snowstar {

ModulesI::ModulesI() {
}

ModulesI::~ModulesI() {
}

int	ModulesI::numberOfModules(const Ice::Current& /* current */) {
	return _repository.numberOfModules();
}

ModuleNameList	ModulesI::getModuleNames(const Ice::Current& /* current */) {
	ModuleNameList	result = _repository.moduleNames();
	// suppress names that should not be published
	ModuleNameList::iterator	i;
	i = find(result.begin(), result.end(), std::string("mock2"));
	if (i != result.end()) result.erase(i);
	i = find(result.begin(), result.end(), std::string("net"));
	if (i != result.end()) result.erase(i);
	i = find(result.begin(), result.end(), std::string("nice"));
	if (i != result.end()) result.erase(i);
	// make sure the module list is sorted
	std::stable_sort(result.begin(), result.end());
	return result;
}

DriverModulePrx	ModulesI::getModule(const std::string& modulename,
	const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for module '%s'",
		modulename.c_str());
	return snowstar::createProxy<DriverModulePrx>(std::string("drivermodule/") + modulename, current, false);
}

} // namespace snowstar
