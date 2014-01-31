/*
 * ModulesI.cpp -- modules servant implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ModulesI.h>
#include <AstroLoader.h>
#include <DriverModuleI.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Communicator.h>

namespace snowstar {

std::vector<std::string>	ModulesI::modulenames() {
	std::vector<std::string>	modules = repository.moduleNames();
	return modules;
}

int	ModulesI::numberOfModules(const Ice::Current& ice) {
	return repository.numberOfModules();
}

ModuleNameSequence	ModulesI::getModuleNames(const Ice::Current& current) {
	return modulenames();
}

typedef IceUtil::Handle<DriverModuleI>	DriverModuleIPtr;

DriverModulePrx	ModulesI::getModule(const std::string& modulename,
			const Ice::Current& current) {
	// ensure that the module exists
	if (!repository.contains(modulename)) {
		throw NotFound("module does not exist");
	}

	// now get a module ptr for it
	astro::module::ModulePtr	module
		= repository.getModule(modulename);
	std::string	name = std::string("module:") + modulename;

	// get adapter/communicator information
	Ice::ObjectAdapterPtr	adapter = current.adapter;
	Ice::CommunicatorPtr	ic = adapter->getCommunicator();

	// build the server
	DriverModuleIPtr	servant = new DriverModuleI(module);
	DriverModulePrx proxy = DriverModulePrx::uncheckedCast(
			adapter->add(servant, ic->stringToIdentity(name)));
	
	return proxy;
}

} // namespace snowstar
