/*
 * imagerepo.cpp -- image repository client tool
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <typeinfo>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <iostream>
#include <includes.h>
#include <AstroConfig.h>
#include <AstroDebug.h>

using namespace astro::config;

namespace astro {

int	command_add(const std::vector<std::string>& arguments) {
	std::cerr << "'add' command not implemented" << std::endl;
	return EXIT_FAILURE;
}	

int	command_list(const std::vector<std::string>& arguments) {
	std::cerr << "'list' command not implemented" << std::endl;
	return EXIT_FAILURE;
}	

int	command_get(const std::vector<std::string>& arguments) {
	std::cerr << "'get' command not implemented" << std::endl;
	return EXIT_FAILURE;
}	

/**
 * \brief Usage function in 
 */
void	usage(const char *progname) {
	std::cerr << "usage:" << std::endl;
	std::cerr << progname << " [ options ] <server> add <image.fits>";
	std::cerr << std::endl;
	std::cerr << progname << " [ options ] <server> list" << std::endl;
	std::cerr << progname << " [ options ] <server> get <id> <image.fits>";
	std::cerr << std::endl;
	std::cerr << "add, list and retrieve images in image server <server>";
	std::cerr << std::endl;
	std::cerr << "optoins:" << std::endl;
	std::cerr << "  -c,--config=<cfg>    use configuration file <cfg>";
	std::cerr << std::endl;
	std::cerr << "  -d,--debug           increase debug level" << std::endl;
	std::cerr << "  -h,--help            display this help message";
	std::cerr << std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function of the imagerepo program
 */
int	main(int argc, char *argv[]) {
	std::string	configfile;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh", longopts,
		&longindex))) {
		switch (c) {
		case 'c':
			configfile = std::string(optarg);
			Configuration::set_default(configfile);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
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

	// next argument must be the image server name
	if (arguments.size() < 2) {
		std::cerr << "no image server name argument" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	imservername = arguments[0];
	std::string	command = arguments[1];
	if (command == "add") {
		return command_add(arguments);
	}
	if (command == "list") {
		return command_list(arguments);
	}
	if (command == "get") {
		return command_get(arguments);
	}

	// get the image server from the configuration
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
