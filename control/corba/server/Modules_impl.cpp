/*
 * Modules_impl.cpp -- Modules servant implementation
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Modules_impl.h"
#include "DriverModule_impl.h"
#include <OrbSingleton.h>
#include <AstroLoader.h>
#include <AstroDebug.h>
#include <algorithm>

namespace Astro {

/**
 * \brief retrieve a list of modules that are available via CORBA
 */
std::vector<std::string>	Modules_impl::modulenames() {
	// get a list of all modules
	std::vector<std::string>	modules = repository.moduleNames();

	// remove the net module, we don't want to have that in the server
	std::vector<std::string>::iterator	k
		= find(modules.begin(), modules.end(), std::string("net"));
	if (k != modules.end()) {
		modules.erase(k);
	}

	// return the remaining modules
	return modules;
}

/**
 * \brief Get the number of available modules.
 */
::CORBA::Long	Modules_impl::numberOfModules() {
	return modulenames().size();
}

/**
 * \brief Get the list of module names.
 */
Astro::Modules::ModuleNameSequence*	Modules_impl::getModuleNames() {
	std::vector<std::string>	modules = modulenames();

	// convert the vector into a ModuleSequence
	Astro::Modules::ModuleNameSequence	*result
		= new Astro::Modules::ModuleNameSequence();
	result->length(modules.size());
	std::vector<std::string>::const_iterator	i;
	int	j = 0;
	for (i = modules.begin(); i != modules.end(); i++) {
		(*result)[j++] = ::CORBA::string_dup(i->c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created list with %d names",
		result->length());
	return result;
}

/**
 * \brief load a certain module and return a reference to it
 *
 * \param _name	name of the module
 */
Astro::_objref_DriverModule     *Modules_impl::getModule(const char *_name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for module %s", _name);
	std::string	name(_name);

	// get the available modules names
	std::vector<std::string>	names = modulenames();
	if (names.end() == std::find(names.begin(), names.end(), name)) {
		NotFound	notfound;
		notfound.cause = CORBA::string_dup("module not available");
		throw notfound;
	}

	// create an objectid for this driver module
	std::string	oidstr = std::string("module:") + name;
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(oidstr.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "oid: %s", oidstr.c_str());

	// now we create an object reference in the POA for modules, for
	// that we first have to get the POA
	OrbSingleton	orb;
	PoaName	poapath("Modules/DriverModules");
	PortableServer::POA_var	modules_poa = orb.findPOA(poapath);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DriverModules POA found");
	CORBA::Object_var	obj
		= modules_poa->create_reference_with_id(oid,
			"IDL:/Astro/DriverModule");
	return DriverModule::_narrow(obj);
}

} // namespace Astro
