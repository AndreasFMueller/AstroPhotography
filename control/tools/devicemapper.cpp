/*
 * devicemapper.cpp -- command to map devices to short names
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <typeinfo>
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
namespace app {
namespace devicemapper {

bool	verbose = false;
std::string	type("camera");

/**
 * \brief list of device mappingslist of device mappings
 */
int	list_cmd(const std::vector<std::string>& /* arguments */) {
	ConfigurationPtr	configuration = Configuration::get();
	DeviceMapperConfigurationPtr	devicemappers
		= DeviceMapperConfiguration::get(configuration);
	DeviceMapperPtr	devicemapper = devicemappers->devicemapper();
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

	// get a repository
	Repository	repository;

	// get the module
	ModulePtr	module = repository.getModule(modulename);

	// read the device locator from a module
	DeviceLocatorPtr	locator = module->getDeviceLocator();

	// get a list of devices
	std::vector<std::string>	devices = locator->getDevicelist(
		astro::DeviceName::string2type(type));

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
	DeviceMapperConfigurationPtr	devicemappers
		= DeviceMapperConfiguration::get(config);
	DeviceMapperPtr	devicemapper = devicemappers->devicemapper();
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
	if (pairs.has("server")) {
		devmap.servername(pairs("server"));
	}

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
	ConfigurationPtr	configuration = Configuration::get();
	DeviceMapperConfigurationPtr	devicemappers
		= DeviceMapperConfiguration::get(configuration);
	devicemappers->devicemapper()->remove(mapname);
	return EXIT_SUCCESS;
}

/**
 * \brief Display commands 
 */
void	usage(const std::string& progname) {
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::string	p = "    " + Path(progname).basename();
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] scan <module> ..." << std::endl;
	std::cout << p << " [ options ] map <name> <devicename> "
					"[ attr=value ... ]" << std::endl;
	std::cout << p << " [ options ] remove <name>" << std::endl;
	std::cout << std::endl;
	std::cout << "The list command displays a list of device mappings "
		"present in the database." << std::endl;
	std::cout << "The scan command scans the named modules and displays "
		"the devices recoginized" << std::endl;
	std::cout << "by this module." << std::endl;
	std::cout << "The map command creates and updates a map entry. "
			"The <devicename> must" << std::endl;
	std::cout << "always be specified, this is the parameter that might "
			"change when the";
	std::cout << std::endl;
	std::cout << "device is plugged in. The additional attribute value "
			"pairs are normally only";
	std::cout << std::endl;
	std::cout << "set the first time, the attributes 'unit' and "
			"'description' are recognized." << std::endl;
	std::cout << "The remove command removes a named map entry."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "attributes recognized by the map command:" << std::endl;
	std::cout << std::endl;
	std::cout << "  unit             the unit number (currently CCD only)"
		<< std::endl;
	std::cout << "  server           the URL of the server" << std::endl;
	std::cout << "  description      a short description of the component"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -c,--config=<cfg>    use configuration file <cfg>";
	std::cout << std::endl;
	std::cout << "  -d,--debug           increase debug level" << std::endl;
	std::cout << "  -h,--help            display this help message";
	std::cout << std::endl;
	std::cout << "  -s,--server=server   use remote server for device "
			"location";
	std::cout << std::endl;
	std::cout << "  -t,--type=devtype    scan for devices of type <devtype>";
	std::cout << std::endl;
	std::cout << "  -v,--verbose         show more details in repo listing";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "type",	required_argument,	NULL,		't' }, /* 4 */
{ "verbose",	no_argument,		NULL,		'v' }, /* 5 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief The main method of the devicemapper command
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhs:t:v", longopts,
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
			return EXIT_FAILURE;
			break;
		case 't':
			type = optarg;
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
	if (cmd == "help") {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}
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

	// if we get to this point, then the command was not recognized
	std::cerr << "command '" << cmd << "' not known" << std::endl;
	return EXIT_FAILURE;
}

} // namespace devicemapper
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::devicemapper::main>(argc, argv);
}
