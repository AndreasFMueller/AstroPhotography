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
#include "Images_impl.h"
#include <NameService.h>
#include <AstroLoader.h>
#include <OrbSingleton.h>
#include <cassert>
#include "DriverModuleActivator_impl.h"
#include "ImageActivator_impl.h"
#include "TaskQueue_impl.h"
#include "TaskActivator_impl.h"
#include <POABuilder.h>
#include <AstroPersistence.h>
#include <AstroTask.h>
#include <AstroExceptions.h>

namespace astro {

/**
 * \brief usage message for astrod program
 */
void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -dh? ] [ omniorboptions ] [ -b imagedir ] [ -q dbfile ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "omniorboptions    see the omniorb documentation for these options" << std::endl;
	std::cout << "                  you should at least add an option that will allow" << std::endl;
	std::cout << "                  the program to find the COS naming server, something like" << std::endl;
	std::cout << std::endl;
	std::cout << "                      -ORBInitRef NameService=corbaname::localhost" << std::endl;
	std::cout << std::endl;
	std::cout << "                  will do in most cases." << std::endl;
	std::cout << " -d               increase debug level" << std::endl;
	std::cout << " -h, -?           display this help message an exit" << std::endl;
	std::cout << " -b imagedir      directory containing the images taken by the server" << std::endl;
	std::cout << "                  and made available to clients" << std::endl;
	std::cout << " -q dbfile        name of the database file containing persistent" << std::endl;
	std::cout << "                  task state and possibly other parameters" << std::endl;
}

/**
 * \brief Main function for the CORBA server
 */
int	astrod_main(int argc, char *argv[]) {
	debugtimeprecision = 3;
	debuglevel = LOG_DEBUG;
	debugthreads = true;

	// initialize random number generator (used in the simulator)
	srandom(0);

	// initialize CORBA
	Astro::OrbSingleton	orb(argc, argv);

	// parameters
	std::string	databasefile("testdb.db");

	// default operation is in the background
	bool	stay_in_foreground = false;

	// now parse the command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "db:q:h?F")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'b':
			Astro::ImageObjectDirectory::basedir(optarg);
			break;
		case 'q':
			databasefile = std::string(optarg);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		case 'F':
			stay_in_foreground = true;
			break;
		}

	// starting the astro daemon
	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod starting up");

	// go into the background
	if (!stay_in_foreground) {
		pid_t	child = fork();
		if (child < 0) {
			// an error happend
			throw runtime_errno("cannot fork", errno);
		}
		if (child > 0) {
			// we are in the parent, so we should exist now
			exit(EXIT_SUCCESS);
		}
		if (child == 0) {
			// we are in the child, so we can proceed, we just
			// log our success here
			debug(LOG_DEBUG, DEBUG_LOG, 0, "child forked");
		}
	}

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
	POABuilder	pb(root_poa);
	PortableServer::POA_var	modules_poa = pb.build("Modules");

	// create a POA for driver modules
	POABuilderActivator<Astro::DriverModuleActivator_impl>	pb1(modules_poa);
	PortableServer::POA_var	drivermodules_poa = pb1.build("DriverModules",
		new Astro::DriverModuleActivator_impl());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DriverModuleActivator set");

	// create a POA for Camera objects
	POABuilder	pbcamera(drivermodules_poa);
	PortableServer::POA_var	camera_poa
		= pbcamera.build("Cameras");

	// create a POA for Ccd objects
	POABuilder	pbccd(camera_poa);
	PortableServer::POA_var	ccd_poa
		= pbccd.build("Ccds");

	// create a POA for Cooler objects
	POABuilder	pbcooler(ccd_poa);
	PortableServer::POA_var	cooler_poa
		= pbcooler.build("Coolers");

	// create a POA GuiderPort objects
	POABuilder	pbguiderport(camera_poa);
	PortableServer::POA_var	guiderport_poa
		= pbguiderport.build("GuiderPorts");

	// create a POA for FilterWheel objects
	POABuilder	pbfilterwheel(camera_poa);
	PortableServer::POA_var	filterwheel_poa
		= pbfilterwheel.build("FilterWheels");

	// create a POA for Focuser objects
	POABuilder	pbfocuser(drivermodules_poa);
	PortableServer::POA_var	focuser_poa
		= pbfocuser.build("Focusers");
			
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
	astro::guiding::GuiderFactoryPtr
		gfptr(new astro::guiding::GuiderFactory(repository));
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

	// create a POA for guiders
	POABuilder	pbguider(root_poa);
	PortableServer::POA_var	guider_poa
		= pbguider.build("Guiders");

	// create a servant for images
	Astro::Images_impl	*images = new Astro::Images_impl();
	PortableServer::ObjectId_var	imagessid
		= root_poa->activate_object(images);

	// register the Images servant
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Images", "object"));
	nameservice.bind(names, images->_this());

	// a POA for images
	POABuilderActivator<Astro::ImageActivator_impl>	pb2(root_poa);
	PortableServer::POA_var	images_poa = pb2.build("Images",
		new Astro::ImageActivator_impl());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ImageActivator set");

	// create the task queue
	astro::persistence::DatabaseFactory	factory;
	astro::persistence::Database	database
		= factory.get(databasefile);
	astro::task::TaskQueue	taskqueue(database);

	// create the servant for TaskQueue
	Astro::TaskQueue_impl	*taskqueueservant
		= new Astro::TaskQueue_impl(taskqueue);
	PortableServer::ObjectId_var	taskqueuesid
		= root_poa->activate_object(taskqueueservant);

	// register the TaskQueue servant
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("TaskQueue", "object"));
	nameservice.bind(names, taskqueueservant->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task queue servant activated");

	// a POA for Tasks
	POABuilderActivator<Astro::TaskActivator_impl>	pb3(root_poa);
	PortableServer::POA_var	tasks_poa = pb3.build("Tasks",
		new Astro::TaskActivator_impl(database));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TaskActivator set");

	// activate the POA manager
	PortableServer::POAManager_var	pman = root_poa->the_POAManager();
	pman->activate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "poa manager activated");

	// run the orb
	orb.orbvar()->run();
	orb.orbvar()->destroy();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod exiting");
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::astrod_main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "astrod terminated by exception: " << x.what()
			<< std::endl;
	}
	exit(EXIT_FAILURE);
}
