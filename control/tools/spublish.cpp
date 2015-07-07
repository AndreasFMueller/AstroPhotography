/*
 * spublish.cpp -- service publish test client
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <ServiceDiscovery.h>
#include <getopt.h>
#include <includes.h>
#include <thread>

using namespace astro;
using namespace astro::discover;

namespace astro {
namespace app {
namespace publish {

void	usage(const char *progname) {
	std::cout << "usage:" << std::endl;
	std::string	p = "    " + Path(progname).basename();
	std::cout << p << " " << std::endl;
	std::cout << std::endl;
	std::cout << "  -d,--debug        increase debug level" << std::endl;
	std::cout << "  -h,--help         display this help message and exit";
	std::cout << std::endl;
	std::cout << "  -n,--name=<name>  use <name> as the service name";
	std::cout << std::endl;
	std::cout << "  -p,--port=<port>  use <port> as the port number";
	std::cout << std::endl;
}

static struct option	longopts[] = {
	{ "debug",	no_argument,		NULL,	'd' },
	{ "help",	no_argument,		NULL,	'h' },
	{ "port",	required_argument,	NULL,	'p' },
	{ "name",	required_argument,	NULL,	'n' },
	{ NULL,		0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	int	port = 4711;

	std::string	hostname;
	char	h[1024];
	if (0 == gethostname(h, sizeof(h))) {
		hostname = std::string(h, strlen(h));
	}

	while (EOF != (c = getopt_long(argc, argv, "dhp:n:",
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
		case 'p':
			port = std::stoi(optarg);
			break;
		case 'n':
			hostname = std::string(optarg);
			break;
		}

	// additional arguments are service names and ports
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service name %s:%d", hostname.c_str(),
		port);

	// create a service discovery object
	ServicePublisherPtr	sd = ServicePublisher::get(hostname, port);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "publishing object created");
	std::this_thread::sleep_for(std::chrono::seconds(2));

	// publish a service
	for (int i = optind; i < argc; i++) {
		ServiceObject::service_type	t
			= ServiceObject::type_name(argv[i]);
		sd->add(t);
	}
	sd->publish();

	// wait some time
	std::this_thread::sleep_for(std::chrono::seconds(10));

	// remove the service
	for (int i = optind; i < argc; i++) {
		ServiceObject::service_type	t
			= ServiceObject::type_name(argv[i]);
		sd->revoke(t);
	}
	sd->publish();

	// wait some more
	std::this_thread::sleep_for(std::chrono::seconds(1));

	// that's it, done
	return EXIT_SUCCESS;
}

} // namespace sdiscover
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::publish::main>(argc, argv);
}