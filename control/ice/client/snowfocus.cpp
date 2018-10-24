/*
 * snowfocus.cpp -- focusing client 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <focusing.h>
#include <typeinfo>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroDebug.h>
#include <getopt.h>
#include <CommunicatorSingleton.h>
#include <RemoteInstrument.h>
#include <IceConversions.h>
#include <AstroFormat.h>
#include <CommonClientTasks.h>
#include <IceUtil/UUID.h>

using namespace astro::config;
using namespace astro;
using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowfocus {

/**
 * \brief Callback class for the snowfocus program
 *
 * This callback simply displays the callback information received
 */
class FocusCallbackI : public FocusCallback {
public:
	/**
	 * \brief Create a callback object
	 */
	FocusCallbackI() {
	}

	/**
	 * \brief add a point
	 */
	void	addPoint(const FocusPoint& point,
			const Ice::Current& /* current */) {
		std::cout << timeformat("%H:%M:%S ", time(NULL));
		std::cout << point.position << ": " << point.value;
		std::cout << std::endl;
	}

	/**
 	 * \brief change the state
	 */
	void	changeState(FocusState state,
			const Ice::Current& /* current */) {
		std::cout << timeformat("%H:%M:%S ", time(NULL));
		std::cout << "new state: ";
		std::cout << focusingstate2string(state);
		std::cout << std::endl;
	}
};

static void	short_usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "Usage:" << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> start <min> <max>" << std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> info"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> monitor"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> cancel"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> status"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> history"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> repository [ <repo> ]"
		<< std::endl;
}

/**
 * \brief Usage method
 */
static void	usage(const char *progname) {
	short_usage(progname);
	std::cout << "start, monitor, cancel or report the status of a "
		"focusing operation";
	std::cout << std::endl;
	std::cout << "positions <min> and <max>";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--binning=XxY      select XxY binning mode (default 1x1)"
		<< std::endl;
	std::cout << " -c,--config=<cfg>     use configuration from file <cfg>";
	std::cout << std::endl;
	std::cout << " -d,--debug            increase debug level" << std::endl;
	std::cout << " -e,--exposure=<e>     set exposure time to <e>";
	std::cout << std::endl;
	std::cout << " -f,--filter=<f>       use filter numbered <f>, ignored "
		"if the instrument has";
	std::cout << std::endl;
	std::cout << "                       no filter wheel";
	std::cout << std::endl;
	std::cout << " -h,--help             display this help message and exit";
	std::cout << std::endl;
	std::cout << " -m,--method=<m>       method to use to estimate focus "
		"quality: either";
	std::cout << std::endl;
	std::cout << "                       brenner, fwhm, measure, default is brenner";
	std::cout << std::endl;
	std::cout << " --rectangle=<rec>     expose only a subrectangle as "
		"specified by <rec>.";
	std::cout << std::endl;
	std::cout << "                       <rec> must be of the form";
	std::cout << std::endl;
	std::cout << "                       widthxheight@(xoffset,yoffset)";
	std::cout << std::endl;
	std::cout << " -R,--remote           assume that the server name describes a service that" << std::endl;
	std::cout << "                       cannot be discovered via ZeroConf" << std::endl;
	std::cout << " -s,--steps=<s>        subdivide the interval in <s> "
		"steps";
	std::cout << std::endl;
	std::cout << " -t,--temperature=<t>  cool ccd to temperature <t>, "
		"ignored if the instrument";
	std::cout << std::endl;
	std::cout << "                       has no cooler";
	std::cout << std::endl;
}

/**
 * \brief long options
 */
static struct option	longopts[] = {
{ "binning",		required_argument,	NULL,	'b' }, /*  0 */
{ "config",		required_argument,	NULL,	'c' }, /*  1 */
{ "debug",		no_argument,		NULL,	'd' }, /*  2 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  3 */
{ "filter",		required_argument,	NULL,	'f' }, /*  4 */
{ "help",		no_argument,		NULL,	'h' }, /*  5 */
{ "method",		required_argument,	NULL,	'm' }, /*  6 */
{ "rectangle",		required_argument,	NULL,	'r' }, /*  7 */
{ "remote",		no_argument,		NULL,	'R' }, /*  5 */
{ "steps",		required_argument,	NULL,	's' }, /*  8 */
{ "temperature",	required_argument,	NULL,	't' }, /*  9 */
{ NULL,			0,			NULL,    0  }
};

/**
 * \brief Display the history of the last focusing run
 */
static void	show_history(const FocusHistory& history) {
	std::for_each(history.begin(), history.end(),
		[](const FocusPoint& point) {
			std::cout << point.position << ": " << point.value;
			std::cout << std::endl;
		}
	);
}

volatile bool	signal_received = false;
void	handler(int /* sig */) {
	signal_received = true;
}

/**
 * \brief Main function for the snowfocus program
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowfocus");
	snowstar::CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = cs.get();

	bool	remote = false;
	int	steps = 10;
	double	exposuretime = 1.0;
	double	temperature = std::numeric_limits<double>::quiet_NaN();
	std::string	binning;
	std::string	frame;
	std::string	filtername;
	astro::focusing::Focusing::method_type	method
		= astro::focusing::Focusing::BRENNER;

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:de:f:hi:m:r:Rt:",
		longopts, &longindex)))
		switch (c) {
		case 'b':
			binning = optarg;
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposuretime = std::stod(optarg);
			break;
		case 'f':
			filtername = optarg;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'm':
			method = astro::focusing::Focusing::string2method(optarg);
			break;
		case 'r':
			frame = optarg;
			break;
		case 'R':
			remote = true;
			break;
		case 's':
			steps = std::stoi(optarg);
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// the next argument is the command
	if (argc <= optind) {
		short_usage(argv[0]);
		throw std::runtime_error("missing service argument");
	}
	std::string	argument(argv[optind++]);
	if ("help" == argument) {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}
	astro::ServerName	servername(argument);
	if (argc <= optind) {
		short_usage(argv[0]);
		throw std::runtime_error("missing instrument name argument");
	}

	// make sure the server offers instruments and guiding
	if (!remote) {
		astro::discover::ServiceDiscoveryPtr     sd
			= astro::discover::ServiceDiscovery::get();
		astro::discover::ServiceObject  so
			= sd->find(sd->waitfor(argument));
		if (!so.has(astro::discover::ServiceSubset::INSTRUMENTS)) {
			std::cerr << "service '" << argument;
			std::cerr << "' does not offer focusing service";
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
		if (!so.has(astro::discover::ServiceSubset::FOCUSING)) {
			std::cerr << "service '" << argument;
			std::cerr << "' does not offer focusing service";
			std::cerr << std::endl;
			return EXIT_FAILURE;
		}
	}

	std::string	instrumentname(argv[optind++]);
	if (argc <= optind) {
		short_usage(argv[0]);
		throw std::runtime_error("missing command argument");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", command.c_str()); 

	// get a proxy for the instruments
	Ice::ObjectPrx	base = ic->stringToProxy(
				servername.connect("Instruments"));
	InstrumentsPrx	instruments = InstrumentsPrx::checkedCast(base);

	// get the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
		throw std::runtime_error("instrument name not set");
	}
	RemoteInstrument	instrument(instruments, instrumentname);

	// make sure the server names for focuser and ccd are identical
	astro::ServerName	targetserver
					= instrument.servername(InstrumentCCD);
	if (targetserver != instrument.servername(InstrumentFocuser)) {
		throw std::runtime_error("ccd and focuser are on different "
			"servers");
	}

	// get the device names
	CcdPrx	ccdprx = instrument.ccd();
	std::string	ccdname = ccdprx->getName();
	FocuserPrx	focuserprx = instrument.focuser();
	std::string	focusername = focuserprx->getName();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd: %s focuser: %s", ccdname.c_str(),
		focusername.c_str());

	// first get a connection to the server
	Ice::ObjectPrx	fbase = ic->stringToProxy(
				targetserver.connect("FocusingFactory"));
	FocusingFactoryPrx	focusingfactory
		= FocusingFactoryPrx::checkedCast(fbase);

	// get the focusing interface
	FocusingPrx	focusing = focusingfactory->get(ccdname, focusername);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a focusing proxy");

	// creating a callback
	Ice::ObjectPtr	callback = new FocusCallbackI();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	focusing->ice_getConnection()->setAdapter(adapter.adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback installed");

	// handle the simple commands
	if (command == "help") {
		short_usage(argv[0]);
		return EXIT_SUCCESS;
	}

	if (command == "status") {
		std::cout << "status: ";
		FocusState	state = focusing->status();
		std::cout << focusingstate2string(state);
		switch (state) {
		case FocusFOCUSED:
			std::cout << " ";
			std::cout << focusing->getFocuser()->current();
			break;
		default:
			break;
		}
		std::cout << std::endl;
		return EXIT_SUCCESS;
	}

	if (command == "info") {
		std::cout << "position: ";
		std::cout << focuserprx->current();
		std::cout << ", range: ";
		std::cout << focuserprx->min();
		std::cout << ", ";
		std::cout << focuserprx->max();
		std::cout << std::endl;
		return EXIT_SUCCESS;
	}

	if (command == "history") {
		show_history(focusing->history());
		return EXIT_SUCCESS;
	}

	if (command == "monitor") {
		std::cout << "current status: ";
		std::cout << focusingstate2string(focusing->status());
		std::cout << std::endl;
		focusing->registerCallback(ident);
		signal(SIGINT, handler);
		while (!signal_received) {
			sleep(1);
		}
		focusing->unregisterCallback(ident);
		return EXIT_SUCCESS;
	}

	if (command == "cancel") {
		focusing->cancel();
		std::cout << "cancel command sent" << std::endl;
		return EXIT_SUCCESS;
	}

	if (command == "repository") {
		if (optind < argc) {
			std::string	reponame(argv[optind]);
			focusing->setRepositoryName(reponame);
		} else {
			std::string	reponame
				= focusing->getRepositoryName();
			if (reponame.size() > 0) {
				std::cout << "repository: " << reponame;
				std::cout << std::endl;
			} else {
				std::cout << "repository not set" << std::endl;
			}
		}
		return EXIT_SUCCESS;
	}

	// throw exception for unknown commands
	if (command != "start") {
		short_usage(argv[0]);
		throw std::runtime_error("unknown command");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "executing start command");

	// make sure temperature is set
	CoolerTask	coolertask(instrument, temperature);
	coolertask.stop_on_exit(true);
	coolertask.wait();

	// next two arguments are the interval boundaries
	if ((argc - optind) < 2) {
		short_usage(argv[0]);
		throw std::runtime_error("missing interval arguments");
	}
	int	min = std::stoi(argv[optind++]);
	int	max = std::stoi(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interval [%d,%d]", min, max);
	if (min >= max) {
		short_usage(argv[0]);
		throw std::runtime_error("not an interval");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing in interval [%d,%d]",
		min, max);

	// ensure that focuser is ready
	FocusState	state = focusing->status();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current state = %d", state);
	if ((state == FocusMOVING) && (state == FocusMEASURING)) {
		short_usage(argv[0]);
		throw std::runtime_error("already focusing");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser available");

	// set up the exposure
	astro::camera::Exposure	exposure;
	exposure.purpose(astro::camera::Exposure::focus);
	exposure.exposuretime(exposuretime);
	if (binning.size() > 0) {
		exposure.mode(astro::image::Binning(binning));
	}
	exposure.shutter(astro::camera::Shutter::OPEN);
	if (frame.size() > 0) {
		exposure.frame(astro::image::ImageRectangle(frame));
	}

	// set up the focusing
	focusing->setSteps(steps);
	focusing->setMethod(convert(method));
	focusing->setExposure(convert(exposure));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing set up %d steps, method %s",
		steps,
		astro::focusing::Focusing::method2string(method).c_str());

	// start the focusing process
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting between %d and %d", min, max);
	focusing->start(min, max);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing started, status: %d",
		focusing->status());

	// wait for the process to complete
	bool	completed = false;
	signal(SIGINT, handler);
	do {
		sleep(1);
		switch (focusing->status()) {
        	case FocusIDLE:
        	case FocusMOVING:
        	case FocusMEASURING:
			break;
        	case FocusFOCUSED:
        	case FocusFAILED:
			completed = true;
			break;
		}
	} while ((!completed) && (!signal_received));
	if (completed) {
		std::cout << "final focus position: " << focuserprx->current();
		std::cout << std::endl;

		// display the history
		show_history(focusing->history());
	} else {
		std::cout << "focusing incomplete" << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

} // namespace snowfocus
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowfocus::main>(argc, argv);
}

