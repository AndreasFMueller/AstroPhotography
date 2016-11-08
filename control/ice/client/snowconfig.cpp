/*
 * snowconfig.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <AstroUtils.h>
#include <types.h>
#include <includes.h>
#include <AstroConfig.h>

namespace snowstar {
namespace app {
namespace snowconfig {

bool	verbose = false;

static void	short_usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <server> get <domain> <section> <name>" << std::endl;
	std::cout << p << " [ options ] <server> set <domain> <section> <name> <value>" << std::endl;
	std::cout << p << " [ options ] <server> remove <domain> <section> <name>" << std::endl;
	std::cout << p << " [ options ] <server> list [ <domain> [ <section> ] ]" << std::endl;
}

/**
 * \brief Usage function for the snowconfig programm
 */
static void	usage(const char *progname) {
	short_usage(progname);
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from <cfg>"
		<< std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -v,--verbose       display information more verbosely"
		<< std::endl;
	std::cout << " -h,--help          display this help message and exit"
		<< std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument, 	NULL,		'c' },
{ "debug",	no_argument, 		NULL,		'd' },
{ "help",	no_argument, 		NULL,		'h' },
{ "verbose",	no_argument,		NULL,		'v' },
{ NULL,		0,			NULL,		 0  }
};

static int	get_command(ConfigurationPrx configuration,
			const std::list<std::string>& arguments) {
	if (arguments.size() != 3) {
		std::cerr << "wrong number of arguments" << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationKey	key;
	std::list<std::string>::const_iterator	i = arguments.begin();
	key.domain = *i++;
	key.section = *i++;
	key.name = *i;
	if (verbose) {
		std::cout << key.domain << ".";
		std::cout << key.section << ".";
		std::cout << key.name << " ";
	}
	std::cout << configuration->get(key).value << std::endl;
	return EXIT_SUCCESS;
}

static int	set_command(ConfigurationPrx configuration,
			const std::list<std::string>& arguments) {
	if (arguments.size() != 4) {
		std::cerr << "wrong number of arguments" << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationItem	entry;
	std::list<std::string>::const_iterator	i = arguments.begin();
	entry.domain = *i++;
	entry.section = *i++;
	entry.name = *i++;
	entry.value = *i++;
	configuration->set(entry);
	return EXIT_SUCCESS;
}

static int	remove_command(ConfigurationPrx configuration,
			const std::list<std::string>& arguments) {
	if (arguments.size() != 3) {
		std::cerr << "wrong number of arguments" << std::endl;
		return EXIT_FAILURE;
	}
	ConfigurationKey	key;
	std::list<std::string>::const_iterator	i = arguments.begin();
	key.domain = *i++;
	key.section = *i++;
	key.name = *i++;
	try {
		configuration->remove(key);
	} catch (const NotFound x) {
		std::cerr << "not found: " << x.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}


void	show(ConfigurationList list) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list %d entries", list.size());
	for_each(list.begin(), list.end(),
		[](const ConfigurationItem& entry) {
			std::cout << entry.domain << ".";
			std::cout << entry.section << ".";
			std::cout << entry.name << " ";
			std::cout << entry.value << std::endl;
		}
	);
}

static int	list_command(ConfigurationPrx configuration,
			const std::list<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list with %d arguments",
		arguments.size());
	switch (arguments.size()) {
	case 0:
		show(configuration->list());
		return EXIT_SUCCESS;
	case 1: {
		std::string	domain = *arguments.begin();
		show(configuration->listDomain(domain));
		return EXIT_SUCCESS;
		}
	case 2: {
		std::list<std::string>::const_iterator	i = arguments.begin();
		std::string	domain = *i;
		i++;
		std::string	section = *i;
		show(configuration->listSection(domain, section));
		return EXIT_SUCCESS;
		}
	default:
		std::cerr << "wrong number of arguments" << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int	help_command(const char *progname) {
	usage(progname);
	return EXIT_SUCCESS;
}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowconfig");
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dhv", longopts,
		&longindex)))
		switch (c) {
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'v':
			verbose = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// must have another argument
	if (optind >= argc) {
		std::cerr << "missing argument" << std::endl;
		short_usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	serverargument(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "serverargument = %s",
		serverargument.c_str());

	if (serverargument == "help") {
		return help_command(argv[0]);
	}

	// must have yet another argument
	if (optind >= argc) {
		std::cerr << "missing command" << std::endl;
		short_usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	command(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command = %s", command.c_str());
	std::list<std::string>	arguments;
	while (optind < argc) {
		arguments.push_back(std::string(argv[optind++]));
	}
	if ("help" == command) {
		return help_command(argv[0]);
	}

	// create a sever connection
	astro::ServerName	servername(serverargument);
	Ice::ObjectPrx	base = ic->stringToProxy(
				servername.connect("Configuration"));
	ConfigurationPrx	configuration
		= ConfigurationPrx::checkedCast(base);
	if (!configuration) {
		throw std::runtime_error("cannot connect to remote server");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connected to configuration service");
	
	// now process the various commands
	if ("get" == command) {
		return get_command(configuration, arguments);
	}
	if ("remove" == command) {
		return remove_command(configuration, arguments);
	}
	if ("set" == command) {
		return set_command(configuration, arguments);
	}
	if ("list" == command) {
		return list_command(configuration, arguments);
	}
	std::cerr << "command " << command << " unknown" << std::endl;

	return EXIT_FAILURE;
}

} // namespace snowconfig
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowconfig::main>(argc, argv);
}

