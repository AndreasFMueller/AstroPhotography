/*
 * OrbSingleton.cpp -- Orb stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OrbSingleton.h>
#include <AstroDebug.h>
#include <NameService.h>
#include <stdexcept>
#include <sstream>
#include <AstroUtils.h>

namespace Astro {

//////////////////////////////////////////////////////////////////////
// PoaName implementation
//////////////////////////////////////////////////////////////////////

PoaName::PoaName(const std::string& basename) {
	add(basename);
}

PoaName&	PoaName::add(const std::string& name) {
	std::vector<std::string>	names;
	astro::split<std::vector<std::string> >(name, "/", names);
	std::copy(names.begin(), names.end(), std::back_inserter(*this));
	return *this;
}

std::string	PoaName::toString() const {
	return astro::Concatenator::concat(*this, std::string("/"));
}

std::ostream&	operator<<(std::ostream& out, const PoaName& poaname) {
	return out << poaname.toString();
}

PoaName	PoaName::modules() {
	return PoaName("Modules");
}

PoaName	PoaName::drivermodules() {
	return modules().add("DriverModules");
}

PoaName	PoaName::cameras() {
	return drivermodules().add("Cameras");
}

PoaName	PoaName::guiderports() {
	return cameras().add("GuiderPorts");
}

PoaName	PoaName::filterwheels() {
	return cameras().add("FilterWheels");
}

PoaName	PoaName::ccds() {
	return cameras().add("Ccds");
}

PoaName	PoaName::coolers() {
	return ccds().add("Coolers");
}

PoaName	PoaName::focusers() {
	return drivermodules().add("Focusers");
}

PoaName	PoaName::guiders() {
	return PoaName("Guiders");
}

PoaName	PoaName::images() {
	return PoaName("Images");
}

//////////////////////////////////////////////////////////////////////
// OrbSingleton implementation
//////////////////////////////////////////////////////////////////////

static CORBA::ORB_var	global_orbvar = CORBA::ORB::_nil();

/**
 * \brief Create an ORB reference from the command line
 */
OrbSingleton::OrbSingleton(int& argc, char *argv[]) {
	const char* options[][2] = {
					{ "giopMaxMsgSize", "40000000" },
					{ 0, 0 }
				};

	CORBA::ORB_ptr  orb = CORBA::ORB_init(argc, argv, "omniORB4", options);
	global_orbvar = orb;
	_orbvar = global_orbvar;
	
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got ORB");

	for (int i = 0; i < argc; i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "argv[%d] = %s", i, argv[i]);
	}
}

/**
 * \brief Create a copy of the existing ORB
 */
OrbSingleton::OrbSingleton() {
	// if the orbvar has not been initialized, terrible things should happen
	_orbvar = global_orbvar;
}

/**
 * \brief Destroy this reference
 */
OrbSingleton::~OrbSingleton() {
}

/**
 * \brief Get the Modules reference from the ORB
 */
Modules_var	OrbSingleton::getModules() {
        // get a reference to the naming service
        Astro::Naming::NameService      nameservice(_orbvar);
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
	return modules._retn();
}

/**
 * \brief Get the Images reference from the ORB
 */
Images_var	OrbSingleton::getImages() {
        // get a reference to the naming service
        Astro::Naming::NameService      nameservice(_orbvar);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

        // Next we want to get a reference to the Images object
        Astro::Naming::Names    names;
        names.push_back(Astro::Naming::Name("Astro", "context"));
        names.push_back(Astro::Naming::Name("Images", "object"));
        CORBA::Object_var       obj = nameservice.lookup(names);

        // get a reference to the images interface
        Astro::Images_var      images = Astro::Images::_narrow(obj);
        if (CORBA::is_nil(images)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a images object");
	return images._retn();
}

/**
 * \brief Get 
 */
TaskQueue_var	OrbSingleton::getTaskQueue() {
        // get a reference to the naming service
        Astro::Naming::NameService      nameservice(_orbvar);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

        // Next we want to get a reference to the Images object
        Astro::Naming::Names    names;
        names.push_back(Astro::Naming::Name("Astro", "context"));
        names.push_back(Astro::Naming::Name("TaskQueue", "object"));
        CORBA::Object_var       obj = nameservice.lookup(names);

        // get a reference to the images interface
        Astro::TaskQueue_var      taskqueue = Astro::TaskQueue::_narrow(obj);
        if (CORBA::is_nil(taskqueue)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a taskqueue object");
	return taskqueue._retn();
}

/**
 * \brief Get the GuiderFactory reference
 */
GuiderFactory_var	OrbSingleton::getGuiderfactory() {
        Astro::Naming::NameService      nameservice(_orbvar);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

        // Next we want to get a reference to the Modules object
        Astro::Naming::Names    names;
        names.push_back(Astro::Naming::Name("Astro", "context"));
        names.push_back(Astro::Naming::Name("GuiderFactory", "object"));
        CORBA::Object_var       obj = nameservice.lookup(names);

        // get a reference to the modules interface
        Astro::GuiderFactory_var      guiderfactory = Astro::GuiderFactory::_narrow(obj);
        if (CORBA::is_nil(guiderfactory)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a GuiderFactory object");
	return guiderfactory._retn();
}

/**
 * \brief Get the DeviceLocator for a given module
 */
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
	return locatorvar._retn();
}

/**
 * \brief Find a POA of a given name
 */
PortableServer::POA_var	OrbSingleton::findPOA(const std::vector<std::string>& poaname) {
	CORBA::Object_ptr	obj
		= _orbvar->resolve_initial_references("RootPOA");
	PortableServer::POA_var	poa
		= PortableServer::POA::_narrow(obj);
	for (unsigned int i = 0; i < poaname.size(); i++) {
		poa = poa->find_POA(poaname[i].c_str(), 0);
	}
	return poa._retn();
}

} // namespace Astro
