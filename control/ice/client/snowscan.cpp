/*
 * snowscan.cpp -- command line tool to scan server for devices
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <Ice/Ice.h>
#include <iostream>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <includes.h>
#include <device.h>

namespace snowflake {

/**
 * \brief list of options for the snowscan command
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "port",	required_argument,	NULL,		'p' }, /* 3 */
{ "server",	required_argument,	NULL,		's' }, /* 4 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Usage display method
 */
void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cerr << "usage:" << std::endl;
	std::cerr << std::endl;
	std::cerr << p << " [ options ] modules" << std::endl;
	std::cerr << p << " [ options ] scan <modulename> " << std::endl;
	std::cerr << std::endl;
	std::cerr << "The first command lists all modules available on the "
		"server, while the";
	std::cerr << std::endl;
	std::cerr << "second version retrives the devices available from that "
		"module";
	std::cerr << std::endl;
	std::cerr << "options:" << std::endl;
	std::cerr << " -d,--debug         increase debug level" << std::endl;
	std::cerr << " -h,--help          display help message and exit";
	std::cerr << std::endl;
	std::cerr << " -p,--port=<p>      connect to the server using port <p>,"
		" default is 10000";
	std::cerr << std::endl;
	std::cerr << " -s,--server=<s>    connect to server named <s>, default "
		"is localhost";
	std::cerr << std::endl;
}

/**
 * \brief Implementation of the modules command
 */
int	command_modules(snowstar::ModulesPrx& modules) {
	std::cout << "number of modules: " << modules->numberOfModules();
	std::cout << std::endl;
	std::cout << "modules names:" ;
	snowstar::ModuleNameList	names = modules->getModuleNames();
	std::for_each(names.begin(), names.end(), [](const std::string& name) {
			std::cout << " " << name;
		}
	);
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief List elements of a certain type
 */
void	devicelist(snowstar::DeviceLocatorPrx devicelocator,
	snowstar::devicetype type) {
	snowstar::DeviceNameList	devicenames
		= devicelocator->getDevicelist(type);
	std::for_each(devicenames.begin(), devicenames.end(),
		[](const std::string& name) {
			 std::cout << " " << name;
		}
	);
	std::cout << std::endl;
}

/**
 * \brief Implementation of the scan command
 */
int	command_scan(snowstar::ModulesPrx& modules,
		const std::string& modulename) {
	std::cout << "scanning module " << modulename << std::endl;
	snowstar::DriverModulePrx	module
		= modules->getModule(modulename);
	std::cout << "name:     " << module->getName() << std::endl;
	std::cout << "version:  " << module->getVersion() << std::endl;
	std::cout << "locator:  ";
	std::cout << ((module->hasLocator()) ? "YES" : "NO");
	std::cout << std::endl;
	if (!module->hasLocator()) {
		return EXIT_SUCCESS;
	}
	snowstar::DeviceLocatorPrx	devicelocator
		= module->getDeviceLocator();
	snowstar::DeviceNameList	devicenames;
	std::cout << "cameras:      ";
	devicelist(devicelocator, snowstar::DevCAMERA);
	std::cout << "ccds:         ";
	devicelist(devicelocator, snowstar::DevCCD);
	std::cout << "coolers:      ";
	devicelist(devicelocator, snowstar::DevCOOLER);
	std::cout << "filterwheels: ";
	devicelist(devicelocator, snowstar::DevFILTERWHEEL);
	std::cout << "focuser:      ";
	devicelist(devicelocator, snowstar::DevFOCUSER);
	std::cout << "guiderport:   ";
	devicelist(devicelocator, snowstar::DevGUIDERPORT);
	
	return EXIT_SUCCESS;
}

int	main(int argc, char *argv[]) {
	// connect to the server
	Ice::InitializationData	id;
	Ice::CommunicatorPtr	ic;
	try {
		ic = Ice::initialize(argc, argv, id);
	} catch (...) {
		throw;
	}

	// some variables changed by options
	unsigned short	port = 10000;
	std::string	servername("localhost");

	// parse the remaining command line arguments
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dhs:p:", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'p':
			port = std::stod(optarg);
			break;
		case 's':
			servername = std::string(optarg);
			break;
		}
	}


	// connect to the server
	std::string	connectstring
		= astro::stringprintf("Modules:default -h %s -p %hu",
			servername.c_str(), port);

	Ice::ObjectPrx	base = ic->stringToProxy(connectstring);
	snowstar::ModulesPrx	modules
		= snowstar::ModulesPrx::checkedCast(base);
	if (!modules) {
		throw std::runtime_error("no modules proxy");
	}

	// next argument is the module
	if (argc <= optind) {
		throw std::runtime_error("command argument missing");
	}
	std::string	commandname = argv[optind++];

	if ("modules" == commandname) {
		return command_modules(modules);
	}

	if ("scan" == commandname) {
		if (optind >= argc) {
			throw std::string("missing module name");
		}
		std::string	modulename = argv[optind++];
		return command_scan(modules, modulename);
	}
	return EXIT_FAILURE;
}
	
} // namespace astro

int main(int argc, char *argv[]) {
	try {
		return snowflake::main(argc, argv);
	} catch (const Ice::Exception& icex) {
		std::cerr << "terminated to ICE exception: ";
		std::cerr << icex << std::endl;
	} catch (const std::exception& x) {
		std::cerr << "terminated by ";
		std::cerr << astro::demangle(typeid(x).name());
		std::cerr << ": " << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
