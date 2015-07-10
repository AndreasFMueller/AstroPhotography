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
	std::cout << "  -t,--timeout=<t>  wait for <t> seconds until exiting";
	std::cout << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
	{ "debug",	no_argument,		NULL,	'd' },
	{ "help",	no_argument,		NULL,	'h' },
	{ "timeout",	required_argument,	NULL,	't' },
	{ NULL,		0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	int	timeout = 10;
	while (EOF != (c = getopt_long(argc, argv, "dht:",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			debugthreads = 1;
			debugtimeprecision = 3;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 't':
			timeout = std::stoi(optarg);
			break;
		}

	if (timeout == 0) {
		timeout = 2147483647;
	}

	// create a service discovery object
	ServiceDiscoveryPtr	sd = ServiceDiscovery::get();
	std::this_thread::sleep_for(std::chrono::seconds(timeout));

	// display list of services 
	ServiceDiscovery::ServiceKeySet	s = sd->list();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "services found: %d", s.size());
	std::cout << s;

	// resolve the first service
	if (s.size() == 0) {
		std::cout << "no services found" << std::endl;
		return EXIT_SUCCESS;
	}
	ServiceKey	key = *(sd->list().begin());
	ServiceObject	object = sd->find(key);
	std::cout << object.toString() << std::endl;

	return EXIT_SUCCESS;
}

} // namespace sdiscover
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::discover::main>(argc, argv);
}
