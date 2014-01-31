/*
 * snowflake.cpp -- a test client for the snowstar server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ice/Ice.h>
#include <module.h>
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
			= ic->stringToProxy("Modules:default -p 10000");
		snowstar::ModulesPrx	modules = ModulesPrx::checkedCast(base);
		if (!modules) {
			throw "invalid proxy";
		}
		std::cout << "number of modules: ";
		std::cout << modules->numberOfModules(); 
		std::cout << std::endl;

		std::vector<std::string>	modulenames
			= modules->getModuleNames();
		std::vector<std::string>::const_iterator	i;
		for (i = modulenames.begin(); i != modulenames.end(); i++) {
			std::cout << *i << std::endl;
		}

		// now get a certain module
		DriverModulePrx	module = modules->getModule(modulename);
		std::cout << "Module: " << module->getName() << std::endl;
		
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
