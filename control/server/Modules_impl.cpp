/*
 * Modules_impl.cpp -- Modules servant implementation
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <../idl/device.hh>
#include "Modules_impl.h"
#include "DriverModule_impl.h"
#include <AstroLoader.h>
#include <AstroDebug.h>

namespace Astro {

/**
 * \brief Get the number of available modules.
 */
::CORBA::Long	Modules_impl::numberOfModules() {
	return repository.numberOfModules();
}

/**
 * \brief Get the list of module names.
 */
Astro::Modules::ModuleNameSequence*	Modules_impl::getModuleNames() {
	std::vector<std::string>	modules = repository.moduleNames();
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
 */
Astro::_objref_DriverModule     *Modules_impl::getModule(const char *_name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for module %s", _name);
	std::string	name(_name);
	astro::module::ModulePtr	result;
	// find out whether this module was already loaded
	modulemap_t::const_iterator	i = modulemap.find(name);
	if (modulemap.find(name) != modulemap.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module %s in cache", _name);
		result = i->second;
	} else {
		// load the module and put the module pointer into the map
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new module %s", _name);
		try {
			result = repository.getModule(name);
			result->open();
		} catch (std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "exception: %s", x.what());
			throw x;
		}
		modulemap.insert(make_pair(name, result));
	}

	// turn the ModulePtr into an object reference
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module %s loaded", name.c_str());
	Astro::DriverModule_impl	*drivermodule
		= new Astro::DriverModule_impl(result);
	
	return drivermodule->_this();
}

} // namespace Astro
