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

namespace snowstar {

/**
 * \brief Main function for the Snowstar server
 */
int	main(int argc, char *argv[]) {
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
	while (EOF != (c = getopt(argc, argv, "db:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'b':
			astro::image::ImageDirectory::basedir(optarg);
			break;
		}

	// set up the repository
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices set up");

	astro::image::ImageDirectory	imagedirectory;

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
		adapter->addServantLocator(
			new DeviceServantLocator(repository, imagedirectory),
			"");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "devices servant added");

		// add a servant for images to the image adapter
		object = new ImagesI(imagedirectory);
		adapter->add(object, ic->stringToIdentity("Images"));
		adapter->addServantLocator(
			new ImageLocator(imagedirectory), "image");
		adapter->activate();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "images servant locator added");

		// activate the adapter
		ic->waitForShutdown();
	} catch (const Ice::Exception& ex) {
		std::cerr << "ICE exception: " << ex << std::endl;
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
	try {
		return snowstar::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "exception: " << x.what() << std::endl;
	} catch (...) {
		std::cerr << "unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
