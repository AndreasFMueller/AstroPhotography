/*
 * snowstream.cpp -- retrieve images via the stream interface
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include <AstroConfig.h>
#include <RemoteInstrument.h>
#include <IceConversions.h>

namespace snowstar {
namespace app {
namespace snowstream {

static void	short_usage(const char *progname) {
	std::cout << "Usage: " << std::endl;
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << p << " [ options ] <service> <INSTRUMENT>" << std::endl;
	std::cout << p << " --help      for more information" << std::endl;
}

/**
 * \brief Usage function for the snowstream application
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << p << " [options] <service> <INSTRUMENT>" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -c,--config=<cfg>    use configuration databaes <cfg>"
		<< std::endl;
	std::cout << " -C,--ccd=<ccd>       use CCD with index <ccd> (default 0)" << std::endl;
	std::cout << " -d,--debug           increase debug level" << std::endl;
	std::cout << " -h,-?,--help         display this help message"
		<< std::endl;
}

static struct option	longopts[] = {
{ "config",		required_argument,	NULL,	'c' },
{ "ccd",		required_argument,	NULL,	'C' },
{ "debug",		no_argument,		NULL,	'd' },
{ "help",		no_argument,		NULL,	'h' },
{ NULL,			0,			NULL,	 0  }
};

/**
 * \brief Main function of the snowstream application
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowstream");
	snowstar::CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	int	ccd_index = 0;

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "c:C:dh?",
		longopts, &longindex))) {
		switch (c) {
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'C':
			ccd_index = atoi(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	// next argument must be the service
	if (optind >= argc) {
		short_usage(argv[0]);
		throw std::runtime_error("service name missing");
	}
	astro::ServerName	servername(argv[optind++]);

	// next argument must be the instrment name
	if (optind >= argc) {
		short_usage(argv[0]);
		throw std::runtime_error("instrument name missing");
	}
	std::string	instrumentname(argv[optind++]);

	// check the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();

	// check the instrument
	if (0 == instrumentname.size()) {
		short_usage(argv[0]);
		throw std::runtime_error("instrument name not set");
	}
	Ice::ObjectPrx	base = ic->stringToProxy(
				servername.connect("Instruments"));
	InstrumentsPrx	instruments = InstrumentsPrx::checkedCast(base);

	// create the remote instrument
	RemoteInstrument	ri(instruments, instrumentname);

	// get the Ccd
	snowstar::CcdPrx	ccd = ri.ccd(ccd_index);

	return EXIT_FAILURE;
}

} // namespace snowstream
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowstream::main>(argc, argv);
}
