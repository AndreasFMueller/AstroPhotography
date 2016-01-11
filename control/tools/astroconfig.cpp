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
	std::cout << p << " [ options ] { get | set | delete } domain section name [ value ]" << std::endl;
	std::cout << p << " [ options ] { list } domain [ section [ name ]]" << std::endl;
	std::cout << std::endl;
	std::cout << "Get, set or delete configuration variables in domain "
		"(currently only" << std::endl;
	std::cout << "'global' is valid), identified by the section and "
		"the name." << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] server list" << std::endl;
	std::cout << p << " [ options ] server add <name> <url> <info>" << std::endl;
	std::cout << p << " [ options ] server remove <name>" << std::endl;
	std::cout << std::endl;
	std::cout << "list, add or remove information about available servers"
		<< std::endl;
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
}

/**
 * \brief help command
 */
int	command_help(const std::vector<std::string>& /* arguments */) {
	usage("astroconfig");
	return EXIT_SUCCESS;
}

/**
 * \brief set a global configuration variable
 */
int	command_set_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 5) {
		std::cerr << "not enough arguments for set command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationPtr	configuration = Configuration::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting value %s",
		arguments[4].c_str());
	configuration->setglobal(arguments[2], arguments[3], arguments[4]);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the set command
 */
int	command_set(const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set command");
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_set_global(arguments);
	}
	std::cerr << "domain " << domain << "  not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the get global command
 */
int	command_get_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for get command";
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
	std::cerr << "domain " << domain << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the global delete command
 */
int	command_delete_global(const std::vector<std::string>& arguments) {
	if (arguments.size() < 4) {
		std::cerr << "not enough arguments for delete command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	try {
		ConfigurationPtr	configuration = Configuration::get();
		configuration->removeglobal(arguments[2], arguments[3]); 
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
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_delete_global(arguments);
	}
	std::cerr << "domain " << domain << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the list global command
 */
int	command_list_global(const std::vector<std::string>& /* arguments */) {
	std::list<ConfigurationEntry>	entries
		= Configuration::get()->globallist();
	std::for_each(entries.begin(), entries.end(),
		[](ConfigurationEntry entry) {
			std::cout << entry.section;
			std::cout << "\t";
			std::cout << entry.name;
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
	std::list<ImageRepoInfo>	repoinfolist = imagerepos->listrepo();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d ImageRepoInfo objects",
		repoinfolist.size());
	std::for_each(repoinfolist.begin(), repoinfolist.end(),
		[](const ImageRepoInfo& repoinfo) {
			std::cout << stringprintf("%-8.8s %s %s\n",
				repoinfo.reponame.c_str(),
				repoinfo.database.c_str(),
				repoinfo.directory.c_str());
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
		imagerepos->removerepo(arguments[2]);
		return EXIT_SUCCESS;
	}
	std::cerr << "unknown subcommand " << arguments[1] << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the server commands
 */
int	command_server(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "no server sub command" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	subcommand = arguments[1];
	ConfigurationPtr	configuration = Configuration::get();
	ServerConfigurationPtr	servers
		= ServerConfiguration::get(configuration);
	if (subcommand == "list") {
		std::list<ServerInfo>	l = servers->listservers();
		std::for_each(l.begin(), l.end(),
			[](const ServerInfo& s) {
				std::cout << s.name() << std::endl;
			}
		);
		return EXIT_SUCCESS;
	}
	if (arguments.size() < 3) {
		std::cerr << "missing server name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	name = arguments[2];
	if (subcommand == "show") {
		ServerInfo	s = servers->server(name);
		std::cout << "Name: " << s.name() << std::endl;
		std::cout << "URL:  " << (std::string)s.servername()
			<< std::endl;
		std::cout << "Info: " << s.info() << std::endl;
		return EXIT_SUCCESS;
	}
	if (subcommand == "remove") {
		servers->removeserver(name);
		return EXIT_SUCCESS;
	}
	if (subcommand == "add") {
		if (arguments.size() < 4) {
			std::cerr << "mandatory arguments missing" << std::endl;
			return EXIT_FAILURE;
		}
		std::string	url = arguments[3];
		ServerInfo	si(name, ServerName(url));
		if (arguments.size() >= 5) {
			si.info(arguments[4]);
		}
		servers->addserver(si);
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/**
 * \brief 
 */
int	command_list(const std::vector<std::string>& arguments) {
	if (arguments.size() < 2) {
		std::cerr << "not enough arguments for get command";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}
	std::string	domain = arguments[1];
	if (domain == "global") {
		return command_list_global(arguments);
	}
	std::cerr << "domain " << domain << " not implemented" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief main method of the astroconfig program
 */
int	main(int argc, char *argv[]) {
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
	if (verb == "server") {
		return command_server(arguments);
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
