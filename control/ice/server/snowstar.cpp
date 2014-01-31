/*
 * snowstar.cpp -- main program for the snow star server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ice/Ice.h>
#include <cstdlib>
#include <iostream>
#include <ModulesI.h>
#include <AstroDebug.h>

namespace snowstar {

/**
 * \brief Main function for the Snowstar server
 */
int	main(int argc, char *argv[]) {
	int	status = EXIT_SUCCESS;
	Ice::CommunicatorPtr	ic;
	try {
		ic = Ice::initialize(argc, argv);
	} catch (...) {
		throw;
	}

	// parse the command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// initialize servants
	try {
		Ice::ObjectAdapterPtr	adapter
			= ic->createObjectAdapterWithEndpoints("ModulesAdapter",
				"default -p 10000");
		Ice::ObjectPtr	object = new ModulesI;
		adapter->add(object, ic->stringToIdentity("Modules"));
		adapter->activate();
		ic->waitForShutdown();
	} catch (const Ice::Exception& ex) {
		std::cerr << "ICE exception: " << ex << std::endl;
		status = EXIT_FAILURE;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		status = EXIT_FAILURE;
	}
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
