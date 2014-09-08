/*
 * astroconfig.cpp -- manage the configuration
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <typeinfo>
#include <AstroDebug.h>
#include <includes.h>

using namespace astro::config;

namespace astro {

/**
 * \brief Table of options
 */
static struct option	longopts[] = {
/* name         argument?               int*    	int */
{ "config",	required_argument,	NULL,		'c' }, /* 0 */
{ "debug",	no_argument,		NULL,		'd' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief usage message
 */
void	usage(const char *progname) {
	std::cerr << "usage: " << progname << " [ options ]" << std::endl;
	std::cerr << "options:" << std::endl;
	std::cerr << "  -c,--config=<configfile>     use configuration from <configfile>" << std::endl;
	std::cerr << "  -d,--debug                   increase debug level";
	std::cerr << std::endl;
	std::cerr << "  -h,--help                    show this help message";
	std::cerr << std::endl;
}

/**
 * \brief
 */
int	command_set_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 5) {
		std::cerr << "not enough arguments for set command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	configuration = Configuration::get();
	configuration->setglobal(arguments[2], arguments[3], arguments[4]);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the set command
 */
int	command_set(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_set_global(arguments);
	}
	std::cerr << "command not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the get global command
 */
int	command_get_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for set command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	try {
		ConfigurationPtr	configuration = Configuration::get();
		std::cout << configuration->global(arguments[2],
					arguments[3]); 
		std::cout << std::endl;
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "not found: "  << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the get command
 */
int	command_get(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_get_global(arguments);
	}
	std::cerr << "command not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief main method of the astroconfig program
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

	ConfigurationPtr	configuration = Configuration::get();

	// remaining arguments are 
	std::vector<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}

	if (0 == arguments.size()) {
		std::cerr << "not engough arguments" << std::endl;
		return EXIT_FAILURE;
	}

	std::string	verb = arguments[0];
	
	if (verb == "get") {
		return command_get(arguments);
	}
	if (verb == "set") {
		return command_set(arguments);
	}
	
	std::cerr << "command " << verb << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
}
