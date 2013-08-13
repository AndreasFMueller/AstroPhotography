/*
 * astroclient.cpp -- a simple client to test the corba server
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "../idl/device.hh"
#include <includes.h>
#include <iostream>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {

int	main(int argc, char *argv[]) {
	// get an orb reference, also removes the ORB arguments from
	// the command line
	CORBA::ORB_ptr	orb = CORBA::ORB_init(argc, argv, "omniORB4");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got ORB");

	// parse the command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// get a reference to the naming service
	CORBA::Object_var	initServ;
	try {
		initServ = orb->resolve_initial_references("NameService");
	} catch (CORBA::ORB::InvalidName& x) {
		std::cerr << "no naming service" << std::endl;
		throw std::runtime_error("no naming service");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

	CosNaming::NamingContext_var	rootContext;
	rootContext = CosNaming::NamingContext::_narrow(initServ);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service root context");

	// Next we want to get a reference to the Modules object
	CORBA::Object_var	obj;

	// prepare the path for object lookup in the root context
	CosNaming::Name	name;
	name.length(2);
	name[0].id = (const char *)"Astro";
	name[0].kind = (const char *)"context";
	name[1].id = (const char *)"Modules";
	name[1].kind = (const char *)"object";
	try {
		obj = rootContext->resolve(name);
	} catch (CosNaming::NamingContext::NotFound& ex) {
		std::cerr << "context not found" << std::endl;
		return EXIT_FAILURE;
	} catch (CORBA::TRANSIENT& ex) {
		return EXIT_FAILURE;
	} catch (CORBA::SystemException& ex) {
		return EXIT_FAILURE;
	}

	// get a reference to the modules interface
	Astro::Modules_var	modules = Astro::Modules::_narrow(obj);
	if (CORBA::is_nil(modules)) {
		throw std::runtime_error("nil object reference");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");

	// convert the object into a Modules object
	std::cout << "number of modules: " << modules->numberOfModules()
		<< std::endl;

	// get the list of all modules, and display it
	Astro::Modules::ModuleNameSequence_var	names
		= modules->getModuleNames();
	for (int i = 0; i < names->length(); i++) {
		std::cout << "module " << i << ": " << names[i] << std::endl;
	}

	// that's it, we are done
	exit(EXIT_SUCCESS);
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "astroclient terminated by exception: "
			<< x.what() << std::endl;
	}
}
