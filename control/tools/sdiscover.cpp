/*
 * sdiscover.cpp -- service discover test client
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <ServiceDiscovery.h>
#include <getopt.h>
#include <includes.h>
#include <thread>

using namespace astro;
using namespace astro::discover;

namespace astro {
namespace app {
namespace discover {

void	usage(const char *progname) {
	std::cout << "usage:" << std::endl;
	std::string	p = "    " + Path(progname).basename();
	std::cout << p << " " << std::endl;
	std::cout << std::endl;
	std::cout << "  -d,--debug        increase debug level" << std::endl;
	std::cout << "  -h,--help         display this help message and exit";
	std::cout << std::endl;
	std::cout << "                    are 'images', 'tasks', 'guiding' and 'instrument'";
	std::cout << std::endl;
}

static struct option	longopts[] = {
	{ "debug",	no_argument,		NULL,	'd' },
	{ "help",	no_argument,		NULL,	'h' },
	{ NULL,		0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			debugthreads = 1;
			debugtimeprecision = 3;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	// create a service discovery object
	ServiceDiscoveryPtr	sd = ServiceDiscovery::get();
	std::this_thread::sleep_for(std::chrono::seconds(10));
	std::cout << sd->resolve() << std::endl;

	// get a list of services 
	std::set<ServiceObject>	s = sd->resolve();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "services found: %d", s.size());

	return EXIT_SUCCESS;
}

} // namespace sdiscover
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::discover::main>(argc, argv);
}
