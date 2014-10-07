/*
 * snowstar.cpp -- main program for the snow star server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ice/Ice.h>
#include <Ice/Properties.h>
#include <Ice/Initialize.h>
#include <cstdlib>
#include <iostream>
#include <DevicesI.h>
#include <ImagesI.h>
#include <AstroDebug.h>
#include <DeviceServantLocator.h>
#include <ImageLocator.h>
#include <AstroTask.h>
#include <TaskQueueI.h>
#include <TaskLocator.h>
#include <GuiderFactoryI.h>
#include <GuiderLocator.h>
#include <ModulesI.h>
#include <DriverModuleLocator.h>
#include <DeviceLocatorLocator.h>
#include <FocusingFactoryI.h>
#include <FocusingLocator.h>

namespace snowstar {

/**
 * \brief Main function for the Snowstar server
 */
int	snowstar_main(int argc, char *argv[]) {
	int	status = EXIT_SUCCESS;

	// get properties from the command line
	Ice::PropertiesPtr	props;
	Ice::CommunicatorPtr	ic;
	try {
		props = Ice::createProperties(argc, argv);
		props->setProperty("Ice.MessageSizeMax", "65536"); // 64 MB
		Ice::InitializationData	id;
		id.properties = props;
		ic = Ice::initialize(id);
	} catch (...) {
		throw;
	}

	// parse the command line
	int	c;
	std::string	databasefile("testdb.db");
	while (EOF != (c = getopt(argc, argv, "db:q:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'b':
			astro::image::ImageDirectory::basedir(optarg);
			break;
		case 'q':
			databasefile = std::string(optarg);
			break;
		}

	// set up the repository
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices set up");

	// create image directory
	astro::image::ImageDirectory	imagedirectory;

	// create database and task queue
	astro::persistence::DatabaseFactory	dbfactory;
	astro::persistence::Database	database
		= dbfactory.get(databasefile);
	astro::task::TaskQueue	taskqueue(database);

	// create guider factory
	astro::guiding::GuiderFactory	guiderfactory(repository, database);

	// initialize servant
	try {
		// create the adapter
		Ice::ObjectAdapterPtr	adapter
			= ic->createObjectAdapterWithEndpoints("Astro",
				"default -p 10000");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adapters created");

		// add a servant for devices to the device adapter
		Ice::ObjectPtr	object = new DevicesI(devices);
		adapter->add(object, ic->stringToIdentity("Devices"));
		DeviceServantLocator	*deviceservantlocator
			= new DeviceServantLocator(repository, imagedirectory);
		adapter->addServantLocator(deviceservantlocator, "");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "devices servant added");

		// add a servant for images to the adapter
		object = new ImagesI(imagedirectory);
		adapter->add(object, ic->stringToIdentity("Images"));
		ImageLocator	*imagelocator
			= new ImageLocator(imagedirectory);
		adapter->addServantLocator(imagelocator, "image");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "images servant locator added");

		// add a servant for taskqueue to the adapter
		object = new TaskQueueI(taskqueue);
		adapter->add(object, ic->stringToIdentity("Tasks"));
		TaskLocator	*tasklocator = new TaskLocator(database);
		adapter->addServantLocator(tasklocator, "task");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "task locator added");

		// add a servant for the guider factory
		GuiderLocator	*guiderlocator = new GuiderLocator();
		object = new GuiderFactoryI(database, guiderfactory,
			guiderlocator, imagedirectory);
		adapter->add(object, ic->stringToIdentity("Guiders"));
		adapter->addServantLocator(guiderlocator, "guider");

		// add a servant for the modules
		object = new ModulesI();
		adapter->add(object, ic->stringToIdentity("Modules"));
		DriverModuleLocator	*drivermodulelocator
			= new DriverModuleLocator(repository);
		adapter->addServantLocator(drivermodulelocator, "drivermodule");
		DeviceLocatorLocator	*devicelocatorlocator
			= new DeviceLocatorLocator(repository);
		adapter->addServantLocator(devicelocatorlocator,
				"devicelocator");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Modules servant added");

		// add a servant for Focusing
		object = new FocusingFactoryI();
		adapter->add(object, ic->stringToIdentity("FocusingFactory"));
		FocusingLocator	*focusinglocator = new FocusingLocator();
		adapter->addServantLocator(focusinglocator, "focusing");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Focusing servant added");

		// activate the adapter
		adapter->activate();
		ic->waitForShutdown();
	} catch (const Ice::Exception& ex) {
		std::cerr << "ICE exception: " << ex.what() << std::endl;
		status = EXIT_FAILURE;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		status = EXIT_FAILURE;
	}

	// destroy the communicator
	if (ic) {
		ic->destroy();
	}
	return status;
}

} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::snowstar_main>(argc, argv);
}
