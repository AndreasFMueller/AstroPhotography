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
#include <AstroConfig.h>
#include <repository.h>
#include <RepositoriesI.h>
#include <RepositoryLocator.h>
#include <AstroDiscovery.h>
#include <AstroFormat.h>
#include <InstrumentLocator.h>
#include <InstrumentsI.h>
#include <CommunicatorSingleton.h>

namespace snowstar {

static struct option	longopts[] = {
{ "base",		required_argument,	NULL,	'b' }, /* 0 */
{ "config",		required_argument,	NULL,	'c' }, /* 1 */
{ "debug",		no_argument,		NULL,	'd' }, /* 2 */
{ "database",		required_argument,	NULL,	'q' }, /* 3 */
{ "foreground",		no_argument,		NULL,	'f' }, /* 4 */
{ "help",		no_argument,		NULL,	'h' }, /* 4 */
{ "port",		required_argument,	NULL,	'p' }, /* 5 */
{ "pidfile",		required_argument,	NULL,	'P' }, /* 5 */
{ "sslport",		required_argument,	NULL,	's' }, /* 6 */
{ "name",		required_argument,	NULL,	'n' }, /* 7 */
{ NULL,			0,			NULL,	 0  }, /* 8 */
};

static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage: " << path.basename() << " [ options ]"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug          enable debug mode" << std::endl;
	std::cout << " -h,--help           display this help message and exit"
		<< std::endl;
	std::cout << " -b,--base=imagedir  directory for images" << std::endl;
	std::cout << " -c,--config=configdb  use alternative configuration "
		"database from file" << std::endl;
	std::cout << "                       configdb" << std::endl;
	std::cout << " -f,--foreground       stay in foreground" << std::endl;
	std::cout << " -q,-database=taskdb   task manager database"
		<< std::endl;
	std::cout << " -p,--port=<port>      port to offer the service on"
		<< std::endl;
	std::cout << " -P,--pidfile=<file>   write the process id to <file>, and remove when exiting" << std::endl;
	std::cout << " -s,--sslport=<port>   use SSL enable port <port>"
		<< std::endl;
	std::cout << " -n,--name=<name>      define zeroconf name to use"
		<< std::endl;
}

/**
 * \brief Main function for the Snowstar server
 */
int	snowstar_main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);
	// default debug settings
	debugtimeprecision = 3;
	debugthreads = true;
	bool	foreground = false;

	// resturn status
	int	status = EXIT_SUCCESS;

	// get properties from the command line
	Ice::PropertiesPtr	props;
	Ice::CommunicatorPtr	ic;
	try {
		props = Ice::createProperties(argc, argv);
		props->setProperty("Ice.MessageSizeMax", "65536"); // 64 MB
		props->setProperty("Ice.Plugin.IceSSL", "IceSSL:createIceSSL");
		Ice::InitializationData	id;
		id.properties = props;
		ic = Ice::initialize(id);
	} catch (...) {
		throw;
	}

	// default configuration
	std::string	databasefile("testdb.db");
	std::string	servicename("server");
	std::string	pidfilename(PIDDIR "/snowstar.pid");

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:dfn:p:q:s:",
		longopts, &longindex)))
		switch (c) {
		case 'b':
			astro::image::ImageDirectory::basedir(optarg);
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			foreground = true;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'q':
			databasefile = std::string(optarg);
			break;
		case 'p':
			astro::discover::ServiceLocation::get().port(std::stoi(optarg));
			break;
		case 'P':
			pidfilename = std::string(optarg);
			break;
		case 's':
			astro::discover::ServiceLocation::get().sslport(std::stoi(optarg));
			break;
		case 'n':
			astro::discover::ServiceLocation::get().servicename(std::string(optarg));
			break;
		}

	// go inter the background
	if (!foreground) {
		pid_t	pid = fork();
		if (pid < 0) {
			std::cerr << "fork failed: " << strerror(errno);
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		if (pid > 0) {
			// parent process, just exit
			return EXIT_SUCCESS;
		}
		// if get here, we are in the child process
		setsid();
		if (chdir("/") < 0) {
			std::cerr << "cannot chdir to /: " << strerror(errno);
			std::cerr<< std::endl;
			return EXIT_FAILURE;
		}
		umask(027);
	}
	astro::PidFile	pidfile(pidfilename);

	// determine which service name to use
	astro::discover::ServiceLocation&	location = astro::discover::ServiceLocation::get();
	astro::discover::ServicePublisherPtr	sp
		= astro::discover::ServicePublisher::get(location.servicename(),
			location.port());
	astro::discover::ServicePublisherPtr	sps;
	if (location.ssl()) {
		sps = astro::discover::ServicePublisher::get(
			location.servicename() + "-ssl", location.sslport());
	}

	// set up the repository
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices set up");
	sp->set(astro::discover::ServiceSubset::INSTRUMENTS);
	if (sps) { sps->set(astro::discover::ServiceSubset::INSTRUMENTS); }

	// create image directory
	astro::image::ImageDirectory	imagedirectory;
	sp->set(astro::discover::ServiceSubset::IMAGES);
	if (sps) { sps->set(astro::discover::ServiceSubset::IMAGES); }

	// create database and task queue
	astro::persistence::DatabaseFactory	dbfactory;
	astro::persistence::Database	database
		= dbfactory.get(databasefile);
	astro::task::TaskQueue	taskqueue(database);
	sp->set(astro::discover::ServiceSubset::TASKS);
	if (sps) { sps->set(astro::discover::ServiceSubset::TASKS); }

	// create guider factory
	astro::guiding::GuiderFactory	guiderfactory(repository, database);
	sp->set(astro::discover::ServiceSubset::GUIDING);
	if (sps) { sps->set(astro::discover::ServiceSubset::GUIDING); }

	// publish the service name
	sp->publish();
	if (sps) { sps->publish(); }

	// initialize servant
	try {
		// create the adapter
		std::string	connectstring = astro::stringprintf(
			"default -p %hu", location.port());
		if (location.sslport() > 0) {
			connectstring += astro::stringprintf(" -p %hu:ssl",
				location.sslport());
		}
		Ice::ObjectAdapterPtr	adapter
			= ic->createObjectAdapterWithEndpoints("Astro",
				connectstring);
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

		// add servant locator for device locator
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

		// add a servant for Repositories
		object = new RepositoriesI();
		adapter->add(object, ic->stringToIdentity("Repositories"));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Repositories servant added");
		RepositoryLocator	*repolocator = new RepositoryLocator();
		adapter->addServantLocator(repolocator, "repository");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Repository servant added");

		// add a servant for Instruments
		object = new InstrumentsI();
		adapter->add(object, ic->stringToIdentity("Instruments"));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Instruments servant added");
		InstrumentLocator	*instrumentlocator = new InstrumentLocator();
		adapter->addServantLocator(instrumentlocator, "instrument");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Instrument servant added");

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
