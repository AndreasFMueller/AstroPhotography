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
	CORBA::Object_var	obj
		= orb->resolve_initial_references("RootPOA");
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

	// get a reference to the root context to bind the module reference to
	// a name for the client
	CosNaming::NamingContext_var	rootContext;
	try {
		CORBA::Object_var	obj;
		obj = orb->resolve_initial_references("NameService");
		rootContext = CosNaming::NamingContext::_narrow(obj);
		if (CORBA::is_nil(rootContext)) {
			std::cerr << "failed to narrow root naming context"
				<< std::endl;
			return EXIT_FAILURE;
		}
	} catch (CORBA::NO_RESOURCES&) {
		std::cerr << "omniORB is not configured" << std::endl;
		return EXIT_FAILURE;
	} catch (CORBA::ORB::InvalidName&) {
		std::cerr << "Service required is invalid" << std::endl;
		return EXIT_FAILURE;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "naming service initialized");

	// create a context for Astro
	try {
		// prepare the name for the context
		CosNaming::Name	contextName;
		contextName.length(1);
		contextName[0].id = (const char *)"Astro";
		contextName[0].kind = (const char *)"context";

		// bind the context
		CosNaming::NamingContext_var	context;
		try {
			context = rootContext->bind_new_context(contextName);
		} catch (CosNaming::NamingContext::AlreadyBound& ex) {
			CORBA::Object_var	obj
				= rootContext->resolve(contextName);
			context = CosNaming::NamingContext::_narrow(obj);
			if (CORBA::is_nil(context)) {
				std::cerr << "failed to narrow naming context"
					<< std::endl;
			}
		}

		// register the name
		CosNaming::Name	objectName;
		objectName.length(1);
		objectName[0].id = (const char *)"Modules";
		objectName[0].kind = (const char *)"object";
		try {
			context->bind(objectName, obj);
		} catch (CosNaming::NamingContext::AlreadyBound& ex) {
			context->rebind(objectName, obj);
		}
	} catch (CORBA::TRANSIENT& ex) {
	} catch (CORBA::SystemException& ex) {
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "object now bound");

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
