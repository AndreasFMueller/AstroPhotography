/*
 * astromount.cpp -- get or set position on a mount
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroConfig.h>
#include <AstroLoader.h>
#include <AstroDevice.h>

using namespace astro::config;
using namespace astro::device;
using namespace astro::module;

namespace astro {

int	list_command(astro::module::Devices& devices) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list command");
	Devices::devicelist	l = devices.getDevicelist(DeviceName::Mount);
	std::for_each(l.begin(), l.end(), [](const DeviceName& name) {
			std::cout << name.toString() << std::endl;
		}
	);
	
	return EXIT_SUCCESS;
}

int	help_command() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "help command");
	std::cout << "commands: help, list, get, set" << std::endl;
	std::cout << std::endl;
	std::cout << "help" << std::endl;
	std::cout << "   Display this command help message." << std::endl;
	std::cout << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "   Display a list of all available mount URLs."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "get MOUNT" << std::endl;
	std::cout << "    Receive current position and tracking status of mount with"
		<< std::endl;
	std::cout << "    device name MOUNT" << std::endl;
	std::cout << std::endl;
	std::cout << "set MOUNT RA DEC" << std::endl;
	std::cout << "    Position the mount to right ascension RA and "
		"declination DEC." << std::endl;
	std::cout << "    RA has te be specified in decimal hours, and DEC in "
		"decimal degrees" << std::endl;
	std::cout << "    On most mounts this will only work if the mount has "
		"been calibrated." << std::endl;
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

int	get_command(MountPtr mount) {
	throw std::runtime_error("get implementation missing");
	return EXIT_SUCCESS;
}

int	set_command(MountPtr mount) {
	throw std::runtime_error("set implementation missing");
	return EXIT_SUCCESS;
}

/**
 * \brief Table of options for the astroinstrument
 */
static struct option    longopts[] = {
/* name		argument?		int*		int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		0   }
};

void	usage(const std::string& progname) {
	std::string	prg = std::string("    ") + Path(progname).basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << prg << "[ options ] help" << std::endl;
	std::cout << prg << "[ options ] list" << std::endl;
	std::cout << prg << "[ options ] get MOUNT" << std::endl;
	std::cout << prg << "[ options ] set MOUNT ra dec" << std::endl;
	std::cout << std::endl;
	std::cout << "list mounts, get or set RA and DEC of a mount";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -h,--help          display help message and exit"
		 << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from file <cfg>"
		 << std::endl;
}

int mount_main(int argc, char *argv[]) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount utility");
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			Configuration::set_default(std::string(optarg));
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 1:
			switch (longindex) {
			}
			break;
		}
	}

	// next argument must be the command
	if (argc <= optind) {
		throw std::runtime_error("missing command argument");
	}
	std::string	command(argv[optind++]);

	// call the command specific functions
	if (command == "help") {
		return help_command();
	}

	// the other commands need a repository
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);

	// list command
	if (command == "list") {
		return list_command(devices);
	}

	// other commands need a mount url 
	if (argc <= optind) {
		throw std::runtime_error("missing mount URL");
	}
	DeviceName	mountname(argv[optind++]);
	if (!mountname.hasType(DeviceName::Mount)) {
		throw std::runtime_error("not a mount device name");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount device name: %s",
		mountname.toString().c_str());

	// use the Devices class to get the mount associated with this name

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::mount_main>(argc, argv);
}
