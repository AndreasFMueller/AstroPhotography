/*
 * astroprocess.cpp -- 
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroProcess.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

using namespace astro::process;

namespace astro {
namespace app {
namespace process {

void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage" << std::endl;
	std::cout << std::endl;
	std::cout << "   " << p.basename();
	std::cout << " [ options ] <process.xml>" << std::endl;
	std::cout << std::endl;
	std::cout << "execute the image processing steps specified in the <process.xml>" << std::endl;
	std::cout << "process description file." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug          show debug messages" << std::endl;
	std::cout << "  -h,--help,-?        show this help message and exit"
		<< std::endl;
	std::cout << "  -v,--verbose        show additional information" << std::endl;
	std::cout << "  -n,--net            display the dependency net and exit" << std::endl;
}

// options for the process command
static struct option	longopts[] = {
{ "debug",	no_argument,	NULL,	'd' },
{ "help",	no_argument,	NULL,	'h' },
{ "verbose",	no_argument,	NULL,	'v' },
{ "net",	no_argument,	NULL,	'n' },
{ NULL,		0,		NULL,	 0  }
};

/**
 * \brief Main method for the astroprocess program
 */
int	main(int argc, char *argv[]) {
	bool	netonly = false;
	int	c;
	int	longindex;
	debugthreads = 1;
	while (EOF != (c = getopt_long(argc, argv, "dh?vn",
			longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'n':
			netonly = true;
			break;
		case 'v':
			ProcessingStep::verbose(true);
			break;
		}

	// next argument must be the filename
	if (optind >= argc) {
		std::string	msg("process description file name missing");
		std::cerr << msg << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	filename(argv[optind++]);

	// construct a process and execute it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing '%s'", filename.c_str());
	ProcessorFactory	factory;
	ProcessorNetworkPtr	network = factory(filename);

	// decide on whether to dump the network
	if (ProcessingStep::verbose() || netonly) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dumping the network");
		network->dump(std::cout);
		if (netonly) {
			return EXIT_SUCCESS;
		}
	}

	// execute the network
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start execution");
	network->process();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing complete");

	// that's it
	return EXIT_SUCCESS;
}

} // namespace process
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::process::main>(argc, argv);
}
