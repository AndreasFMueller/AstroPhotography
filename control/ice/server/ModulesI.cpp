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
	return _repository.moduleNames();
}

DriverModulePrx	ModulesI::getModule(const std::string& modulename,
	const Ice::Current& current) {
	return snowstar::createProxy<DriverModulePrx>(std::string("drivermodule/") + modulename, current);
}

} // namespace snowstar
