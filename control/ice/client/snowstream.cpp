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
#include <AstroDebug.h>
#include <CommonClientTasks.h>

namespace snowstar {
namespace app {
namespace snowstream {

/**
 * \brief Stream sink for this application
 */
class StreamSink : public ImageSink {
	std::mutex		_mutex;
	std::condition_variable	_condition;
public:
	StreamSink() {
	}

	void	stop(const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop");
		std::unique_lock<std::mutex>	lock(_mutex);
		_condition.notify_all();
	}

	void	image(const ImageQueueEntry& entry,
			const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new entry: %s",
			convert(entry.exposure0).toString().c_str());
	}

	void	wait() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for condition");
		std::unique_lock<std::mutex>	lock(_mutex);
		_condition.wait(lock);
	}
};

/**
 * \brief short usage function for snowstream application
 */
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

/**
 * \brief Long options for the snowstream program
 */
static struct option	longopts[] = {
{ "binning",		required_argument,	NULL,	'b' },
{ "config",		required_argument,	NULL,	'c' },
{ "ccd",		required_argument,	NULL,	'C' },
{ "debug",		no_argument,		NULL,	'd' },
{ "exposuretime",	required_argument,	NULL,	'e' },
{ "filter",		required_argument,	NULL,	'f' },
{ "frame",		required_argument,	NULL,	 1  },
{ "focus",		required_argument,	NULL,	'F' },
{ "help",		no_argument,		NULL,	'h' },
{ "purpose",		required_argument,	NULL,	'p' },
{ "temperature",	required_argument,	NULL,	't' },
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
	astro::camera::Exposure	exposure;
	unsigned short	focusposition = 0;
	std::string	filtername;
	double	temperature = std::numeric_limits<double>::quiet_NaN();

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:C:deF:f:hp:?t:",
		longopts, &longindex))) {
		switch (c) {
		case 'b':
			exposure.mode(Binning(optarg));
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'C':
			ccd_index = atoi(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposure.exposuretime(atof(optarg));
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'p':
			exposure.purpose(astro::camera::Exposure::string2purpose(optarg));
			break;
		case 'F':
			focusposition = std::stoi(optarg);
			break;
		case 'f':
			filtername = std::string(optarg);
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		case 1:
			exposure.frame(astro::image::ImageRectangle(optarg));
			break;
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

	// check for focuser
	FocuserTask	focusertask(ri, focusposition);

	// check for filter wheel
	FilterwheelTask	filterwheeltask(ri, filtername);

	// check for cooler
	CoolerTask	coolertask(ri, temperature);

	// now wait for all tasks to complete
	focusertask.wait();
	filterwheeltask.wait();
	coolertask.wait();

	// ImageSink to catch the images
	StreamSink	*sink = new StreamSink();
	Ice::ObjectPtr	sinkptr = sink;
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(sinkptr);

	// register the adapter with the server
	ccd->ice_getConnection()->setAdapter(adapter.adapter());
	ccd->registerSink(ident);

	// start the stream
	ccd->startStream(convert(exposure));

	// wait for the sink to terminate (criterion not yet fixed
	sink->wait();

	// stop and unregister the stream
	ccd->stopStream();
	ccd->unregisterSink();
	return EXIT_FAILURE;
}

} // namespace snowstream
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowstream::main>(argc, argv);
}
