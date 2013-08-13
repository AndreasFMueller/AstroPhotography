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
#include "../idl/NameService.h"

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
	Astro::Naming::NameService	nameservice(orb);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got naming service");

	// Next we want to get a reference to the Modules object
	Astro::Naming::Names	names;
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Modules", "object"));
	CORBA::Object_var	obj = nameservice.lookup(names);

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
	Astro::Modules::ModuleNameSequence_var	namelist
		= modules->getModuleNames();
	for (int i = 0; i < (int)namelist->length(); i++) {
		std::cout << "module " << i << ": " << namelist[i] << std::endl;
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
