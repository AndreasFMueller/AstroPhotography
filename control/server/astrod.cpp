/*
 * astrod.cpp -- a server that controls astro cameras and accessories
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdlib.h>
#include <includes.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <iostream>
#include <omniORB4/CORBA.h>
#include "Modules.h"

namespace astro {

int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod starting up");
	// initialize CORBA
	CORBA::ORB_var	orb = CORBA::ORB_init(argc, argv);
	CORBA::Object_var	obj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var	poa = PortableServer::POA::_narrow(obj);

	// create the servant and register it with the ORB
	Astro::Modules_impl	*modules = new Astro::Modules_impl();
	PortableServer::ObjectId_var	mymodulesid
		= poa->activate_object(modules);

	// create a stringified object reference
	obj = modules->_this();
	CORBA::String_var	sior(orb->object_to_string(obj));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "IOR: %s", (char *)sior);

	modules->_remove_ref();

	PortableServer::POAManager_var	pman = poa->the_POAManager();
	pman->activate();

	orb->run();
	orb->destroy();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod exiting");
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "astrod terminated by exception: " << x.what()
			<< std::endl;
	}
	exit(EXIT_FAILURE);
}
