/*
 * snowservers.cpp -- program to scan for servers
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDiscovery.h>
#include <includes.h>

using namespace astro::discover;

namespace snowstar {
namespace app {
namespace servers {

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ NULL,		0,			NULL,		0   }
};

/**
 * \brief usage information of the snowservers program
 *
 * \param progname	name of the program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] [ servictypes ... ] " << std::endl;
	std::cout << std::endl;
	std::cout << "list all servers that offer astro photo services, or all servers that list";
	std::cout << std::endl;
	std::cout << "any of the service types specified as arguments";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          display help message and exit"
		<< std::endl;
	std::cout << std::endl;

}

/**
 * \brief Main function of the snowservers program
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowservers");
	debugthreads = true;
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// remaining arguments are the service classes that we want to see
	std::list<ServiceSubset::service_type>	servicetypes;
	while (optind < argc) {
		std::string	servicetype(argv[optind++]);
		try {
			servicetypes.push_back(ServiceSubset::string2type(
				servicetype));
		} catch (...) {
			std::cerr << "unknown service type: " << servicetype;
			std::cerr << ", ignored" << std::endl;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "only looking for %d services");

	// create a service discover object
	ServiceDiscoveryPtr	sd = ServiceDiscovery::get();

	// find the service keys
	int	counter = 10;
	ServiceDiscovery::ServiceKeySet	keys;
	do {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "displaying the list");
		ServiceDiscovery::ServiceKeySet	sks;
		if (servicetypes.size() > 0) {
			sks = sd->list(servicetypes);
		} else {
			sks = sd->list();
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d keys", sks.size());

		ServiceDiscovery::ServiceKeySet	removedkeys;
		std::set_difference(keys.begin(), keys.end(),
			sks.begin(), sks.end(),
			inserter(removedkeys, removedkeys.begin()));

		std::for_each(removedkeys.begin(), removedkeys.end(),
			[sd](const ServiceKey& k) {
				std::cout << "deleted: ";
				std::cout << k.toString();
				std::cout << std::endl;
			}
		);

		ServiceDiscovery::ServiceKeySet	newkeys;
		std::set_difference(sks.begin(), sks.end(),
			keys.begin(), keys.end(),
			inserter(newkeys, newkeys.begin()));
		
		std::for_each(newkeys.begin(), newkeys.end(),
			[sd](const ServiceKey& k) {
				ServiceObject	so = sd->find(k);
				std::cout << so.toString();
				std::cout << " ";
				std::cout << so.ServiceSubset::toString();
				std::cout << std::endl;
			}
		);

		keys = sks;

		sleep(1);
	} while (counter--);

	return EXIT_SUCCESS;
}

} // namespace servers
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::servers::main>(argc,
			argv);
	return rc;
}


