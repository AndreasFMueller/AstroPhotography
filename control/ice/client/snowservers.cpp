/*
 * snowservers.cpp -- program to scan for servers
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <includes.h>

namespace snowstar {
namespace app {
namespace servers {

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "help",	no_argument,		NULL,		'h' },
{ NULL,		0,			NULL,		0   }
};

static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] " << std::endl;
	std::cout << std::endl;
	std::cout << "list all servers that offer astro photo services";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -h,--help          display help message and exit"
		<< std::endl;
	std::cout << " -s,--server=<s>    connect to server named <s>, default "
		"is localhost" << std::endl;
	std::cout << std::endl;

}

int	main(int argc, char *argv[]) {
	debug_set_ident("snowservers");
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
		}
	}


	return EXIT_SUCCESS;
}

} // namespace servers
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::servers::main>(argc, argv);
}

