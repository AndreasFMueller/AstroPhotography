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
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// next argument must be the IOR
	if (optind >= argc) {
		std::cerr << "no IOR specified" << std::endl;
		return EXIT_FAILURE;
	}
	char	*ior = argv[optind];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "working with %s", ior);

	// get an orb reference
	CORBA::ORB_ptr	orb = CORBA::ORB_init(argc, argv, "omniORB4");

	// getting an object reference
	CORBA::Object_var	obj = orb->string_to_object(ior);

	// get a reference to the modules interface
	Astro::Modules_var	modules = Astro::Modules::_narrow(obj);
	if (CORBA::is_nil(modules)) {
		throw std::runtime_error("nil object reference");
	}

	// convert the object into a Modules object
	std::cout << "number of modules: " << modules->numberOfModules()
		<< std::endl;

	// get the list of all modules
	Astro::Modules::ModuleNameSequence_var	names = modules->getModuleNames();
	for (int i = 0; i < names->length(); i++) {
		std::cout << "module " << i << ": " << names[i] << std::endl;
	}

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
