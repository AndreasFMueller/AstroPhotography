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

using namespace astro::config;

namespace astro {

/**
 * \brief list of device mappingslist of device mappings
 */
int	list_cmd(const std::vector<std::string>& /* arguments */) {
	DeviceMapperPtr	devicemapper = Configuration::get()->devicemapper();
	std::list<DeviceMap>	devices = devicemapper->select();
	std::cout << "name     unit devicename                       server";
	std::cout << std::endl;
	std::for_each(devices.begin(), devices.end(),
		[](const DeviceMap& devicemap) {
			std::cout << stringprintf("%-8.8s ",
				devicemap.name().c_str());
			std::cout << stringprintf("%4d ", devicemap.unitid());
			std::cout << stringprintf("%-32.32s ",
				devicemap.devicename().toString().c_str());
			std::cout << devicemap.servername();
			std::cout << std::endl;
		}
	);
	return EXIT_SUCCESS;
}

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
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief The main method of the devicemapper command
 */
int	main(int argc, char *argv[]) {
	std::string	servername;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhs:", longopts,
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
