/*
 * devicemapper.cpp -- command to map devices to short names
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <typeinfo>
#include <stacktrace.h>
#include <includes.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroLoader.h>
#include <AstroUtils.h>

using namespace astro::config;
using namespace astro::module;
using namespace astro::device;

namespace astro {

std::string	servername;
bool	verbose = false;

/**
 * \brief list of device mappingslist of device mappings
 */
int	list_cmd(const std::vector<std::string>& /* arguments */) {
	DeviceMapperPtr	devicemapper = Configuration::get()->devicemapper();
	std::list<DeviceMap>	devices = devicemapper->select();
	std::cout << "name     devicename                       unit server";
	if (verbose) {
		std::cout << ", description";
	}
	std::cout << std::endl;
	std::for_each(devices.begin(), devices.end(),
		[](const DeviceMap& devicemap) {
			std::cout << stringprintf("%-8.8s ",
				devicemap.name().c_str());
			std::cout << stringprintf("%-32.32s ",
				devicemap.devicename().toString().c_str());
			std::cout << stringprintf("%4d ", devicemap.unitid());
			if(devicemap.servername().size() == 0) {
				std::cout << "(local)";
			} else {
				std::cout << devicemap.servername();
			}
			if (verbose) {
				std::cout << ", " << devicemap.description();
			}
			std::cout << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Auxiliary functor class to scan devices for a single driver module
 */
class scan_module {
public:
	void	operator()(const std::string& modulename);
};

/**
 * \brief Display the contens of a named module
 */
void	scan_module::operator()(const std::string& modulename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scanning module '%s'",
		modulename.c_str());
	if (servername.size() != 0) {
		std::cerr << "remove device mapping currently not supported";
		std::cerr << std::endl;
		throw std::runtime_error("remote device mapping not supported");
	}

	// get a repository
	Repository	repository;

	// get the module
	ModulePtr	module = repository.getModule(modulename);

	// read the device locator from a module
	DeviceLocatorPtr	locator = module->getDeviceLocator();

	// get a list of devices
	std::vector<std::string>	devices = locator->getDevicelist();

	// display the devices
	std::for_each(devices.begin(), devices.end(), 
		[](const std::string& devicename) {
			std::cout << devicename << std::endl;
		}
	);
}

/**
 * \brief Scan the devices offered by modules defined in the arguments
 */
int	scan_cmd(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw std::runtime_error("scan command needs at least one "
			"module name argument");
	}
	std::for_each(arguments.begin() + 1, arguments.end(), scan_module());
	return EXIT_SUCCESS;
}

/**
 * \brief create or update a map
 */
int	map_cmd(const std::vector<std::string> arguments) {
	// check that we have at least the map argument
	if (arguments.size() < 3) {
		throw std::runtime_error("at least map name and device name required");
	}
	std::string	mapname = arguments[1];
	DeviceName	devname = DeviceName(arguments[2]);

	// get the current mapping entry
	ConfigurationPtr	config = Configuration::get();
	DeviceMapperPtr	devicemapper = config->devicemapper();
	DeviceMap	devmap(devname);
	devmap.name(mapname);
	bool	newmapping = false;
	try {
		devmap = devicemapper->find(mapname);
	} catch (...) {
		newmapping = true;
	}

	// set other values from further arguments
	AttributeValuePairs	pairs(arguments, 3);
	if (pairs.has("description")) {
		devmap.description(pairs("description"));
	}
	if (pairs.has("unit")) {
		devmap.unitid(std::stol(pairs("unit")));
	}
	devmap.servername(servername);

	// perform the update
	if (newmapping) {
		devicemapper->add(devmap);
	} else {
		devicemapper->update(mapname, devmap);
	}

	return EXIT_SUCCESS;
}

/**
 * \brief remove a mapping from the device
 */
int	remove_cmd(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		throw std::runtime_error("missing map name");
	}
	std::string	mapname = arguments[1];
	Configuration::get()->devicemapper()->remove(mapname);
	return EXIT_SUCCESS;
}

/**
 * \brief Display commands 
 */
void	usage(const std::string& progname) {
	std::cerr << "usage:" << std::endl;
	std::cerr << progname << " [ options ] list" << std::endl;
	std::cerr << "display a list of device mappings" << std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "server",	required_argument,	NULL,		's' }, /* 3 */
{ "verbose",	no_argument,		NULL,		'v' }, /* 4 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief The main method of the devicemapper command
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhs:v", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			break;
		case 's':
			servername = optarg;
			break;
		case 'v':
			verbose = true;
			break;
		case 1:
			switch (longindex) {
			}
			break;
		}
	}

	// build the arguments vector
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	// get the subcommand
	if (arguments.size() < 1) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	cmd = arguments[0];
	if (cmd == "list") {
		return list_cmd(arguments);
	}
	if (cmd == "scan") {
		return scan_cmd(arguments);
	}
	if (cmd == "map") {
		return map_cmd(arguments);
	}
	if (cmd == "remove") {
		return remove_cmd(arguments);
	}

	// 
	return EXIT_FAILURE;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
