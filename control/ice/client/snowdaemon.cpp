/*
 * snowdaemon.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <CommunicatorSingleton.h>
#include <types.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <getopt.h>
#include <AstroUtils.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>

namespace snowstar {
namespace app {
namespace daemon {

/**
 * \brief The "help" command
 */
int	command_help(const std::string& progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <server> help" << std::endl;
	std::cout << p << " [ options ] <server> time" << std::endl;
	std::cout << p << " [ options ] <server> sync" << std::endl;
	std::cout << p << " [ options ] <server> shutdown [ delay ]" << std::endl;
	std::cout << p << " [ options ] <server> system [ delay ]" << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug       increase debug level" << std::endl;
	std::cout << " -h,-?,--help     display this help message an exit";
	std::cout << std::endl;
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief The "time" command, get the remote time
 */
int	command_time(DaemonPrx daemon) {
	try {
		time_t	now = daemon->getSystemTime();
		std::cout << ctime(&now);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "cannot get system time: ";
		std::cerr << x.what();
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief The "sync" command, syncs the servers clock
 */
int	command_sync(DaemonPrx daemon) {
	try {
		time_t	now;
		time(&now);
		daemon->setSystemTime(daemon);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "cannot set system time: ";
		std::cerr << x.what();
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief The "shutdown" command, shuts down the server
 */
int	command_shutdown(DaemonPrx daemon, float delay) {
	try {
		daemon->shutdownServer(delay);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "cannot shutdown the server: ";
		std::cerr << x.what();
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}

/**
 * \brief The "system" command, shuts down the system
 */
int	command_system(DaemonPrx daemon, float delay) {
	try {
		daemon->shutdownSystem(delay);
		return EXIT_SUCCESS;
	} catch (const std::exception& x) {
		std::cerr << "cannot shutdown the system: ";
		std::cerr << x.what();
		std::cerr << std::endl;
	}
	return EXIT_FAILURE;
}

static struct option	longopts[] = {
{ "debug",	no_argument,			NULL,	'd' },
{ "help",	no_argument,			NULL,	'h' },
{ NULL,		0,				NULL,	 0  }
};

/**
 * \brief Main method for the snowdaemon programm
 *
 * \param argc	number of arguments
 * \param argv	argument strings
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowdameon");
	CommunicatorSingleton	communicator(argc, argv);
	int	c;
	int	longindex;
	astro::ServerName	servername;
	while (EOF != (c = getopt_long(argc, argv, "dh?", longopts,
		&longindex))) 
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			return command_help(argv[0]);
		default:
			throw std::runtime_error("unknown option");
		}

	// next comes the command
	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	std::string	command(argv[optind++]);

	// handle the help command
	if (command == "help") {
		return command_help(argv[0]);
	}
	servername = astro::ServerName(command);

	if (argc <= optind) {
		throw std::runtime_error("command missing");
	}
	command = std::string(argv[optind++]);

	if (command == "help") {
		return command_help(argv[0]);
	}

	// we need a remote daemon proxy
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(servername.connect("Daemon"));
	DaemonPrx	daemon = DaemonPrx::checkedCast(base);

	// handle more interesting commands
	if (command == "time") {
		return command_time(daemon);
	}
	if (command == "sync") {
		return command_sync(daemon);
	}

	// the following commands may have an additional dealy parameter
	float	delay = 0;
	if (optind < argc) {
		delay = std::stod(argv[optind++]);
	}
	if (command == "shutdown") {
		return command_shutdown(daemon, delay);
	}
	if (command == "system") {
		return command_system(daemon, delay);
	}

	return EXIT_FAILURE;
}

} // namespace daemon
} // namespace app
} // namespace snowstar

int     main(int argc, char *argv[]) {
        int     rc = astro::main_function<snowstar::app::daemon::main>(argc,
                        argv);
        snowstar::CommunicatorSingleton::release();
        return rc;
}

