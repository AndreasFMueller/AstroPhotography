/*
 * OrbSingleton.cpp -- Orb stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OrbSingleton.h>
#include <AstroDebug.h>
#include <NameService.h>
#include <stdexcept>

namespace Astro {

static CORBA::ORB_var	global_orbvar = CORBA::ORB::_nil();

OrbSingleton::OrbSingleton(int argc, char *argv[]) {
	const char* options[][2] = {
					{ "giopMaxMsgSize", "40000000" },
					{ 0, 0 }
				};

	CORBA::ORB_ptr  orb = CORBA::ORB_init(argc, argv, "omniORB4", options);
	global_orbvar = orb;
	orbvar = global_orbvar;
	
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got ORB");
}

OrbSingleton::OrbSingleton() {
	// if the orbvar has not been initialized, terrible things should happen
	orbvar = global_orbvar;
}

OrbSingleton::~OrbSingleton() {
}

Modules_var	OrbSingleton::getModules() {
	        // get a reference to the naming service
        Astro::Naming::NameService      nameservice(orbvar);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

        // Next we want to get a reference to the Modules object
        Astro::Naming::Names    names;
        names.push_back(Astro::Naming::Name("Astro", "context"));
        names.push_back(Astro::Naming::Name("Modules", "object"));
        CORBA::Object_var       obj = nameservice.lookup(names);

        // get a reference to the modules interface
        Astro::Modules_var      modules = Astro::Modules::_narrow(obj);
        if (CORBA::is_nil(modules)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");
	return modules;
}

DeviceLocator_var	OrbSingleton::getDeviceLocator(
				const std::string& modulename) {
	Modules_var	modules = getModules();
	
	// get the Module with the right name
	Astro::DriverModule_var	drivermodule
		= modules->getModule(modulename.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a DriverModule reference");

	// get the device locator from the module
	Astro::DeviceLocator_ptr	locator
		= drivermodule->getDeviceLocator();
	Astro::DeviceLocator_var	locatorvar = locator;
	return locatorvar;
}

} // namespace Astro
