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
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <includes.h>
#include <algorithm>

using namespace astro;
using namespace astro::config;
using namespace astro::project;

namespace astro {
namespace app {
namespace config {

static bool	removecontents = false;

/**
 * \brief Table of options
 */
static struct option	longopts[] = {
/* name         argument?               int*    	int */
{ "config",		required_argument,	NULL,		'c' }, /* 0 */
{ "debug",		no_argument,		NULL,		'd' }, /* 1 */
{ "help",		no_argument,		NULL,		'h' }, /* 2 */
{ "remove-contents",	no_argument,		NULL,		'r' }, /* 3 */
{ NULL,		0,				NULL,		0   }
};

/**
 * \brief usage message
 */
static void	usage(const char *progname) {
	Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << std::endl;
	std::cout << "display a help message about the astrconfig command";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] { get | set | delete } <domain> <section> <name> [ <value> ]" << std::endl;
	std::cout << p << " [ options ] { list } <domain> [ <section> [ <name> ]]" << std::endl;
	std::cout << std::endl;
	std::cout << "Get, set or delete configuration variables in domain <domain>, "
		<< std::endl;
	std::cout << "identified by <section> and <name>." << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] imagerepo list" << std::endl;
	std::cout << p << " [ options ] imagerepo add <reponame> <directory>";
	std::cout << std::endl;
	std::cout << p << " [ options ] imagerepo remove <reponame>";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "list, add or delete image repositores" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -c,--config=<configfile>     use configuration from <configfile>" << std::endl;
	std::cout << "  -d,--debug                   increase debug level";
	std::cout << std::endl;
	std::cout << "  -h,--help                    show this help message";
	std::cout << std::endl;
	std::cout << "  -r,--remove-contents         remove the contents of "
		"the repository when removing it";
	std::cout << std::endl;
}

/**
 * \brief help command
 */
int	command_help(const std::vector<std::string>& /* arguments */) {
	usage("astroconfig");
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the set command
 */
int	command_set(const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set command");
	if (arguments.size() < 5) {
		std::cerr << "not enough arguments for set command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	configuration = Configuration::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting value %s",
		arguments[4].c_str());
	configuration->set(arguments[1], arguments[2], arguments[3],
		arguments[4]);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the get command
 */
int	command_get(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	try {
		ConfigurationPtr	configuration = Configuration::get();
		std::cout << configuration->get(arguments[1], arguments[2],
					arguments[3]); 
		std::cout << std::endl;
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "not found: "  << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the delete command
 */
int	command_delete(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for delete command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	try {
		ConfigurationPtr	config = Configuration::get();
		config->remove(arguments[1], arguments[2], arguments[3]); 
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "not found: "  << x.what() << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the list global command
 */
int	command_list(const std::vector<std::string>& arguments) {
	std::list<ConfigurationEntry>	entries;
	ConfigurationPtr	config = Configuration::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list command with %d arguments",
		arguments.size());
	switch (arguments.size()) {
	case 0:
		throw std::runtime_error("command missing");
	case 1:
		entries = config->list();
		break;
	case 2:
		entries = config->list(arguments[1]);
		break;
	case 3:
	default:
		entries = config->list(arguments[1], arguments[2]);
		break;
	}

	std::for_each(entries.begin(), entries.end(),
		[](const ConfigurationEntry& entry) {
			std::cout << entry.toString();
			std::cout << "\t";
			std::cout << entry.value;
			std::cout << "\n";
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief List all repositories
 */
int	list_repo() {
	ConfigurationPtr	config = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(config);
	std::list<ImageRepoInfo>	repoinfolist = imagerepos->listrepo(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d ImageRepoInfo objects",
		repoinfolist.size());
	std::for_each(repoinfolist.begin(), repoinfolist.end(),
		[](const ImageRepoInfo& repoinfo) {
			std::cout << stringprintf("%-8.8s %s %s %s\n",
				repoinfo.reponame.c_str(),
				repoinfo.database.c_str(),
				repoinfo.directory.c_str(),
				repoinfo.hidden ? "hidden" : "visible");
		}
	);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the image repository commands
 */
int	command_imagerepo(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "no image repo sub command" << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	configuration = Configuration::get();
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(configuration);
	if (arguments[1] == "add") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add repo command");
		if (arguments.size() < 4) {
			std::cerr << "not enough arguments for add command";
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		std::string	reponame = arguments[2];
		std::string	directory = arguments[3];
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add repo '%s' in '%s'",
			reponame.c_str(), directory.c_str());
		struct stat	sb;
		if (stat(directory.c_str(), &sb) < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "adding directory %s",
				directory.c_str());
			if (mkdir(directory.c_str(), 0777) < 0) {
				std::string	msg = astro::stringprintf(
					"cannot create directory %s: %s",
					directory.c_str(), strerror(errno));
				throw std::runtime_error(msg);
			}
		}
		imagerepos->addrepo(reponame, directory);
		return EXIT_SUCCESS;
	}
	if (arguments[1] == "list") {
		return list_repo();
	}
	if (arguments[1] == "remove") {
		imagerepos->removerepo(arguments[2], removecontents);
		return EXIT_SUCCESS;
	}
	std::cerr << "unknown subcommand " << arguments[1] << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief main method of the astroconfig program
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhr", longopts,
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
			break;
		case 'r':
			removecontents = true;
			break;
		case 1:
			switch (longindex) {
			}
			break;
		default:
			throw std::runtime_error("unknown option");
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
	if (verb == "help") {
		return command_help(arguments);
	}
	if (verb == "get") {
		return command_get(arguments);
	}
	if (verb == "set") {
		return command_set(arguments);
	}
	if (verb == "delete") {
		return command_delete(arguments);
	}
	if (verb == "list") {
		return command_list(arguments);
	}
	if (verb == "imagerepo") {
		return command_imagerepo(arguments);
	}
	
	std::cerr << "command " << verb << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

} // namespace config
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::config::main>(argc, argv);
}
