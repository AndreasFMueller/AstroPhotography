/*
 * snowflake.cpp -- a test client for the snowstar server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ice/Ice.h>
#include <device.h>
#include <cstdlib>
#include <iostream>
#include <AstroDebug.h>

using namespace snowstar;

namespace snowflake {

int	main(int argc, char *argv[]) {
	int	status = EXIT_FAILURE;
	Ice::CommunicatorPtr	ic;
	try {
		ic = Ice::initialize(argc, argv);
	} catch (...) {
		throw;
	}

	// parse command line
	int	c;
	std::string	modulename;
	while (EOF != (c = getopt(argc, argv, "dm:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'm':
			modulename = optarg;
			break;
		}

	try {
		Ice::ObjectPrx	base
			= ic->stringToProxy("Devices:default -h othello -p 10000");
		snowstar::DevicesPrx	devices = DevicesPrx::checkedCast(base);
		if (!devices) {
			throw "invalid proxy";
		}
		std::cout << "Cameras:" << std::endl;
		std::vector<std::string>	devicenames
			= devices->getDevicelist(DevCAMERA); 
		std::vector<std::string>::const_iterator	i;
		for (i = devicenames.begin(); i != devicenames.end(); i++) {
			std::cout << *i << std::endl;
		}
		
		status = EXIT_SUCCESS;
	} catch (const Ice::Exception& icex) {
		std::cerr << icex << std::endl;
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
	}
	if (ic) {
		ic->destroy();
	}
	return status;
}

} // namespace snowflake

int	main(int argc, char *argv[]) {
	try {
		return snowflake::main(argc, argv);
	} catch (...) {
		std::cerr << "unknown exception" << std::endl;
	}
}
