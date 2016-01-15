/*
 * snowao.cpp -- client to handle the exercise the adaptive optics unit
 *
 * (c) Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroConfig.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <Ice/Ice.h>
#include <CommunicatorSingleton.h>
#include <RemoteInstrument.h>
#include <device.h>
#include <camera.h>
#include <iostream>

using namespace snowstar;
using namespace astro::config;

namespace snowstar {
namespace app {
namespace snowao {

static void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] <service> <INSTRUMENT> [ <x> <y> ]" << std::endl;
	std::cout << std::endl;
	std::cout << "get the adaptive optics position or set it, if the <x> <y> arguments are set" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -c,--config=<conf>   use configuration file <conf>"
		<< std::endl;
	std::cout << "  -d,--debug           increase debug level" << std::endl;
	std::cout << "  -h,--help            display this help message"
		<< std::endl;
}

static struct option	longopts[] = {
{ "config",		required_argument,	NULL,	'c' }, /* 0 */
{ "debug",		no_argument,		NULL,	'd' }, /* 1 */
{ "help",		no_argument,		NULL,	'h' }, /* 2 */
{ NULL,			0,			NULL,	0   }
};

int	main(int argc, char *argv[]) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "snowao main has %d arguments", argc);
	snowstar::CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "+c:dh",
		longopts, &longindex))) {
		switch (c) {
		case 'c':
			Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	// server and instrument name
	if (optind >= argc) {
		throw std::runtime_error("server name missing");
	}
	astro::ServerName	servername(argv[optind++]);
	if (optind >= argc) {
		throw std::runtime_error("instrument name missing");
	}
	std::string	instrumentname(argv[optind++]);

	// get the configuration
	ConfigurationPtr	config = Configuration::get();

	// get a proxy for instruments
	Ice::ObjectPrx	base = ic->stringToProxy(
				servername.connect("Instruments"));
	InstrumentsPrx	instruments = InstrumentsPrx::checkedCast(base);

	// create a remote instrument
	RemoteInstrument	ri(instruments, instrumentname);

	// get the adaptiveoptics unit
	snowstar::AdaptiveOpticsPrx	ao = ri.adaptiveoptics(0);

	// if there are more arguments, then this is a set command,
	// otherwise it is a read command
	if (optind >= argc) {
		Point	p = ao->get();
		std::cout << astro::stringprintf("AO position: %.3f, %.3f",
			p.x, p.y) << std::endl;
		return EXIT_SUCCESS;
	}

	// ok, this is set, so we need two more arguments
	Point	position;
	if (optind >= argc) {
		throw std::runtime_error("no <x> argument");
	}
	position.x = std::stod(argv[optind++]);
	if (optind >= argc) {
		throw std::runtime_error("no <y> argument");
	}
	position.y = std::stod(argv[optind++]);
	ao->set(position);

	return EXIT_SUCCESS;
}

} // namespace snowao
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowao::main>(argc, argv);
}
