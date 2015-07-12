/*
 * snowinstrument.cpp -- client to manage instruments
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <includes.h>
#include <AstroConfig.h>
#include <tasks.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>
#include <AstroFormat.h>
#include <AstroConfig.h>
#include <AstroProject.h>
#include <AstroIO.h>
#include <ServiceDiscovery.h>

namespace snowstar {
namespace app {
namespace snowinstrument {

bool	verbose = false;

/**
 * \brief Usage function for the snowtask program
 */
void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          show this help and exit" << std::endl;
	std::cout << " -v,--verbose       verbose mode" << std::endl;
}

/**
 * \brief Options for the snowtask program
 */
static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' }, /*  1 */
{ "help",	no_argument,		NULL,		'h' }, /*  2 */
{ "verbose",	no_argument,		NULL,		'v' }, /*  3 */
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief Main function for the snowtask program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	debugthreads = true;

	// parse command line options
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?v",
			longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'v':
			verbose = true;
			break;
		}

	// the next argument is the name of the service
	if (optind >= argc) {
		std::cerr << "missing service name argument" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	name = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service: %s", name.c_str());
	astro::discover::ServiceDiscoveryPtr	discovery
		= astro::discover::ServiceDiscovery::get();
	std::this_thread::sleep_for(std::chrono::seconds(3));
	while (!discovery->has(name)) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	astro::discover::ServiceObject	object = discovery->find(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "host: %s, port: %d",
		object.host().c_str(), object.port());
	return EXIT_FAILURE;
}

} // namespace snowtask
} // namespace app
} // namespace snowtar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowinstrument::main>(argc, argv);
}
