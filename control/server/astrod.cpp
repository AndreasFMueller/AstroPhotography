/*
 * astrod.cpp -- a server that controls astro cameras and accessories
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <includes.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <iostream>
#include <omniORB4/CORBA.h>
#include "Modules_impl.h"
#include "GuiderFactory_impl.h"
#include <NameService.h>
#include <AstroLoader.h>
#include <OrbSingleton.h>
#include <cassert>

namespace astro {

int	main(int argc, char *argv[]) {
	debugtimeprecision = 3;
	debuglevel = LOG_DEBUG;

	// initialize CORBA
	Astro::OrbSingleton	orb(argc, argv);

	// now parse the command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// starting the astro daemon
	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod starting up");

	// get the POA
	CORBA::Object_var	obj
		= orb.orbvar()->resolve_initial_references("RootPOA");
	PortableServer::POA_var	root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initial poa reference");

	// get the naming service
	Astro::Naming::NameService	nameservice(orb);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a name service object");

	// we want a separate child POA for the Modules object, because
	// we want that object reference to be persistent
	PortableServer::LifespanPolicy_var	lifespan
		= root_poa->create_lifespan_policy(PortableServer::TRANSIENT);
	PortableServer::IdAssignmentPolicy_var	assign
		= root_poa->create_id_assignment_policy(PortableServer::USER_ID);
	CORBA::PolicyList	policy_list;
	policy_list.length(2);
	policy_list[0] = PortableServer::LifespanPolicy::_duplicate(lifespan);
	policy_list[1] = PortableServer::IdAssignmentPolicy::_duplicate(assign);
	PortableServer::POA_var	modules_poa
		= root_poa->create_POA("Modules",
			PortableServer::POAManager::_nil(), policy_list);
	lifespan->destroy();
	assign->destroy();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created POA for modules servant");

	// create the servant and register it with the ORB
	Astro::Modules_impl	*modules = new Astro::Modules_impl();
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId("Modules");
	modules_poa->activate_object_with_id(oid, modules);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules servant created");

	// register the object in the name
	Astro::Naming::Names	names;
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Modules", "object"));

	// register the modules object
	nameservice.bind(names, modules->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules bound");

	// create a servant for the guider factory
	astro::module::Repository	repository;
	astro::guiding::GuiderFactoryPtr	gfptr(new astro::guiding::GuiderFactory(repository));
	Astro::GuiderFactory_impl	*guiderfactory
		= new Astro::GuiderFactory_impl(gfptr);
	PortableServer::ObjectId_var	guiderfactorysid
		= root_poa->activate_object(guiderfactory);

	// register the GuiderFactory object
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("GuiderFactory", "object"));
	nameservice.bind(names, guiderfactory->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GuiderFactory object bound");

	// activate the POA manager
	PortableServer::POAManager_var	pman = root_poa->the_POAManager();
	pman->activate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "poa manager activated");
	modules_poa->the_POAManager()->activate();

	// run the orb
	orb.orbvar()->run();
	orb.orbvar()->destroy();

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
