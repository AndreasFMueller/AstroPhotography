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

namespace Astro {

//////////////////////////////////////////////////////////////////////
// PoaName implementation
//////////////////////////////////////////////////////////////////////

std::vector<std::string>	PoaName::split(const std::string& name) const {
	std::vector<std::string>	result;
	std::string	n = name;
	do {
		std::string::size_type	l = n.find('/');
		if (l == std::string::npos) {
			result.push_back(n);
			n = "";
		} else {
			result.push_back(n.substr(0, l));
			n = n.substr(l + 1);
		}
	} while (n.size() > 0);
	return result;
}

PoaName::PoaName(const std::string& basename) {
	add(basename);
}

PoaName&	PoaName::add(const std::string& name) {
	std::vector<std::string>	names = split(name);
	std::vector<std::string>::iterator	i;
	for (i = names.begin(); i != names.end(); i++) {
		push_back(*i);
	}
	return *this;
}

std::string	PoaName::toString() const {
	std::ostringstream	out;
	std::vector<std::string>::const_iterator	i;
	for (i = begin(); i != end(); i++) {
		if (i != begin()) {
			out << "/";
		}
		out << *i;
	}
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const PoaName& poaname) {
	return out << poaname.toString();
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
	return modules;
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
	return locatorvar;
}

/**
 * \brief Find a POA of a given name
 */
PortableServer::POA_var	OrbSingleton::findPOA(const std::vector<std::string>& poaname) {
	CORBA::Object_var	obj
		= _orbvar->resolve_initial_references("RootPOA");
	PortableServer::POA_var	poa
		= PortableServer::POA::_narrow(obj);
	for (int i = 0; i < poaname.size(); i++) {
		poa = poa->find_POA(poaname[i].c_str(), 0);
	}
	return poa;
}

} // namespace Astro
