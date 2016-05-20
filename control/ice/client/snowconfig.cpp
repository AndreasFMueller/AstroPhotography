/*
 * snowconfig.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <types.h>

namespace snowstar {
namespace app {
namespace snowtask {

/**
 * \brief Usage function for the snowconfig programm
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <server> get <domain> <section> <name>" << std::endl;
	std::cout << p << " [ options ] <server> set <domain> <section> <name> <value>" << std::endl;
	std::cout << p << " [ options ] <server> list [ <domain> [ <section> ] ]" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--config=<cfg>  use configuration from <cfg>"
		<< std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          display this help message and exit"
		<< std::endl;
}

static struct option	longopts[] = {
{ "config",	required_argument, 	NULL,		'c' },
{ "debug",	required_argument, 	NULL,		'd' },
{ "help",	required_argument, 	NULL,		'h' },
{ NULL,		0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	debug_set_ident("snowconfig");
	CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:dh", longopts, &longindex)))
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
		}

	

	return EXIT_FAILURE;
}

} // namespace snowtask
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowconfig::main>(argc, argv);
}

