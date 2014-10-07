/*
 * icemount.cpp -- query or position mount
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <cstdlib>
#include <device.h>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <includes.h>
#include <AstroConfig.h>

using namespace snowstar;

namespace icemount {

/**
 * \brief Usage function for the icemount function
 */
void	usage(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") +  path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] get MOUNT" << std::endl;
	std::cout << p << " [ options ] set MOUNT RA DEC" << std::endl;
	std::cout << std::endl;
	std::cout << "get help about the icemount command, list mounts, get "
		"right ascension from" << std::endl;
	std::cout << "the mount, or move the mount to the given coordinates."
		<< std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          display this help message"
		 << std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from file <cfg>"
		 << std::endl;
	std::cout << std::endl;
}

/**
 * \brief array of options
 */
static struct option    longopts[] = {
{ "config",	required_argument,		NULL,	'c' }, /* 1 */
{ "debug",	no_argument,			NULL,	'd' }, /* 2 */
{ "help",	no_argument,			NULL,	'h' }, /* 3 */
{ "server",	required_argument,		NULL,	's' }, /* 4 */
{ NULL,		0,				NULL,	0   }
};

int	command_help() {
	std::cout << "The icemount command understands the ollowing subcommands:";
	std::cout << std::endl;
	std::cout << "help" << std::endl;
	std::cout << "    Display this help" << std::endl;
	std::cout << "list" << std::endl;
	std::cout << "    List all mounts available from the server"
		<< std::endl;
	std::cout << "get MOUNT" << std::endl;
	std::cout << "    Get right ascension and declination from the named "
		"mount. This command" << std::endl;
	std::cout << "    may not work if the mount has not be calibrated yet"
		<< std::endl;
	std::cout << "set MOUNT RA DEC" << std::endl;
	std::cout << "    Move the mount to the specified right ascension and "
		"declination." << std::endl;
	std::cout << "    As with the get command, it will only work if the "
		"mount has already" << std::endl;
	std::cout << "    been calibrated." << std::endl;
	std::cout << std::endl;
}

int	command_list() {
}

int	command_get() {
}

int	command_set() {
}

/**
 * \brief main function 
 */
int	main(int argc, char *argv[]) {
	
	int	c;
	int	longindex;
	astro::ServerName	servername;
	while (EOF != (c = getopt_long(argc, argv, "dhc:", longopts,
		&longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 's':
			servername = astro::ServerName(optarg);
			break;
		}

	// next comes the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help();
	}

	// we need a remote device proxy for all other commands

	// handle the list command
	if (command == "list") {
		return command_list();
	}

	// for the other commands we need the mount name
	if (argc <= optind) {
		throw std::runtime_error("no mount name");
	}
	astro::DeviceName	mountname(argv[optind++]);

	return EXIT_SUCCESS;
}

} // namespace icemount

int	main(int argc, char *argv[]) {
	return astro::main_function<icemount::main>(argc, argv);
}
