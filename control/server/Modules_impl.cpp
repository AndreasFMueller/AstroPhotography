/*
 * Modules_impl.cpp -- Modules servant implementation
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Modules_impl.h"
#include "DriverModule_impl.h"
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

	// make sure the net module is not requested
	if (name == "net") {
		NotFound	notfound;
		notfound.cause = CORBA::string_dup("net module not available "
			"via CORBA");
		throw notfound;
	}

	// prepare the result pointer. We have to do this here, because
	// there are two ways to fill it: from the cache or by creating
	// a new module. 
	astro::module::ModulePtr	result;

	// find out whether this module was already loaded
	modulemap_t::const_iterator	i = modulemap.find(name);
	if (modulemap.find(name) != modulemap.end()) {
		// retrieve the module from the cache
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
			NotFound	notfound;
			notfound.cause = CORBA::string_dup(x.what());
			throw notfound;
		}
		modulemap.insert(make_pair(name, result));
	}

	// turn the ModulePtr into an object reference
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module %s loaded", name.c_str());
	Astro::DriverModule_impl	*drivermodule
		= new Astro::DriverModule_impl(result);
	
	// return the driver module
	return drivermodule->_this();
}

} // namespace Astro
