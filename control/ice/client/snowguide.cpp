/*
 * snowguide.cpp -- command line client to control guiding
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hohschule Rapperswil
 */
#include <AstroUtils.h>
#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <CommunicatorSingleton.h>
#include <CommonClientTasks.h>
#include <getopt.h>
#include <AstroConfig.h>
#include <RemoteInstrument.h>
#include <guider.h>
#include <IceConversions.h>
#include <AstroUtils.h>
#include <AstroImage.h>
#include <ImageCallbackI.h>

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowguide {

bool	verbose = false;
snowstar::ImagePoint	star;
float	focallength = 0;
Exposure	exposure;
std::string	prefix("p");
volatile bool	completed = false;

void	signal_handler(int /* sig */) {
	completed = true;
}

/**
 * \brief usage method
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] calibrate" << std::endl;
	std::cout << p << " [ options ] monitor" << std::endl;
	std::cout << p << " [ options ] images" << std::endl;
	std::cout << p << " [ options ] guide [ <calibrationid> ] " << std::endl;
	std::cout << p << " [ options ] cancel" << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] tracks" << std::endl;
	std::cout << p << " [ options ] history [ trackid ]" << std::endl;
	std::cout << std::endl;
	std::cout << "Operations related to guiding, i.e. calibrating a "
		"guider, starting and";
	std::cout << std::endl;
	std::cout << "monitoring the guding process, and cancelling it.";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--binning=XxY      select XxY binning mode (default "
		"1x1)" << std::endl;
	std::cout << " -c,--config=<cfg>     use configuration from file <cfg>";
	std::cout << std::endl;
	std::cout << " -d,--debug            increase debug level" << std::endl;
	std::cout << " -e,--exposure=<e>     set exposure time to <e>";
	std::cout << std::endl;
	std::cout << " -f,--focallength=<f>  set the focal length of the "
		"instrument";
	std::cout << std::endl;
	std::cout << " -h,--help             display this help message and exit";
	std::cout << std::endl;
	std::cout << " -i,--instrument=<INS> use instrument named INS";
	std::cout << std::endl;
	std::cout << " --rectangle=<rec>     expose only a subrectangle as "
		"specified by <rec>.";
	std::cout << std::endl;
	std::cout << "                       <rec> must be of the form";
	std::cout << std::endl;
	std::cout << "                       widthxheight@(xoffset,yoffset)";
	std::cout << std::endl;
	std::cout << " -t,--temperature=<t>  cool ccd to temperature <t>, "
		"ignored if the instrument";
	std::cout << std::endl;
	std::cout << "                       has no cooler";
	std::cout << std::endl;
	std::cout << " -v,--verbose          enable verbose mode";
	std::cout << std::endl;
}

/**
 * \brief Help command implementation
 */
int	help_command() {
	std::cout <<
"The snowguide program takes the CCD and guiderport defined for" << std::endl <<
"an instrument (specified via the --instrument option) and" << std::endl <<
"builds a guider from them. It understands a number of sub-" << std::endl <<
"commands to control guding via this guider. Subcommands are" << std::endl <<
"specified using the command syntax" << std::endl
<< std::endl << 
"    snowguide [ options ] subcommand" << std::endl
<< std::endl << 
"The folloowing subcommands are available:" << std::endl
<< std::endl
<< std::endl << 
"help" << std::endl << 
"    display this help message and exit" << std::endl
<< std::endl << 
"calibrate [ calibrationid ]" << std::endl <<
"    Use the calibration run specified by <calibrationid> or, if" << std::endl<<
"    <calibrationid> is not specified, start a new calibration" << std::endl <<
"    run. In the latter case the focallength must be specified" << std::endl <<
"    via the -f option." << std::endl
<< std::endl << 
"monitor" << std::endl <<
"    Monitor the guiding process. This subcommand reports all" << std::endl <<
"    state changes and all commands sent to the telescope mount" << std::endl
<< std::endl << 
"guide" << std::endl <<
"    Start guiding with the current calibration id." << std::endl
<< std::endl << 
"cancel" << std::endl <<
"    Cancel the current calibration or guiding process." << std::endl
<< std::endl << 
"list" << std::endl <<
"    List the calibrations available for this guider. The" << std::endl <<
"    calibration id can be used with the calibrate subcommand" << std::endl <<
"    to bring the guider into the calibrated state, a prerequi-" << std::endl <<
"    sitefor guiding" << std::endl
<< std::endl << 
"history" << std::endl <<
"    Display the tracking history of the current guiding run." << std::endl
<< std::endl <<
"For a summary of the options available to all subcommands," << std::endl <<
"run the astroguide command with the --help option."
<< std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the cancel command
 *
 * This command cancels a calibration process or a guiding process.
 */
int	cancel_command(GuiderPrx guider) {
	if (guider->getState() == GuiderCALIBRATING) {
		guider->cancelCalibration();
		return EXIT_SUCCESS;
	}
	if (guider->getState() == GuiderGUIDING) {
		guider->stopGuiding();
		return EXIT_SUCCESS;
	}
	std::cerr << "nothing to cancel, wrong state" << std::endl;
	return EXIT_FAILURE;
}

/**
 * \brief Implementation of the image command
 */
int	images_command(GuiderPrx guider, const std::string& path) {
	// create a Image callback object
	Ice::ObjectPtr	callback = new ImageCallbackI(path, prefix);

	// register the callback with the adapter
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	guider->ice_getConnection()->setAdapter(adapter.adapter());

	// register the image callback with the server
	guider->registerImageMonitor(ident);

	// wait until the callback gets the information that the process
	// completed
	signal(SIGINT, signal_handler);
	while (!completed) {
		sleep(1);
	}

	// unregister the callback before exiting
	guider->unregisterImageMonitor(ident);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the monitor command
 */
int	monitor_command(GuiderPrx /* guider */) {
	// The type of callback to install depends on the current guider state
	return EXIT_SUCCESS;
}

/**
 * \brief A class to display calibration points
 */
class CalibrationPoint_display {
	std::ostream&	_out;
public:
	CalibrationPoint_display(std::ostream& out) : _out(out) { }
	void	operator()(const CalibrationPoint& calpoint);
};

void	CalibrationPoint_display::operator()(const CalibrationPoint& calpoint) {
	_out << "         ";
	_out << astro::stringprintf("%.1f: ", calpoint.t);
	_out << astro::stringprintf("(%f,%f) -> (%f,%f)",
			calpoint.offset.x, calpoint.offset.y,
			calpoint.star.x, calpoint.star.y);
	_out << std::endl;
}

/**
 * \brief A class to display a calibration
 */
class Calibration_display {
	std::ostream&	_out;
public:
	Calibration_display(std::ostream& out) : _out(out) { }
	void	operator()(const Calibration& cal);
};

void	Calibration_display::operator()(const Calibration& cal) {
	// id and timestamp
	_out << astro::stringprintf("%4d: ", cal.id);
	_out << astro::timeformat("%Y-%m-%d %H:%M, ",
		converttime(cal.timeago));
	_out << cal.points.size() << " points" << std::endl;

	// calibration coefficients
	_out << std::string("     ");
	for (int k = 0; k < 3; k++) {
		_out << astro::stringprintf("%12.8f", cal.coefficients[k]);
	}
	_out << std::endl << std::string("     ");
	for (int k = 3; k < 6; k++) {
		_out << astro::stringprintf("%12.8f", cal.coefficients[k]);
	}
	_out << std::endl;

	// calibration points if verbose
	if (verbose) {
		std::for_each(cal.points.begin(), cal.points.end(),
			CalibrationPoint_display(_out));
	}
}

/**
 * \brief Implementation of the list command
 */
int	list_command(GuiderFactoryPrx guiderfactory,
		GuiderDescriptor descriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations from remote server");
	idlist	l = guiderfactory->getCalibrations(descriptor);
	std::cout << "number of calibrations: " << l.size() << std::endl;
	idlist::iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		Calibration	cal = guiderfactory->getCalibration(*i);
		(Calibration_display(std::cout))(cal);
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of calibrate command
 */
int	calibrate_command(GuiderPrx guider, int calibrationid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrationid = %d", calibrationid);
	if (calibrationid > 0) {
		guider->useCalibration(calibrationid);
		return EXIT_SUCCESS;
	}
	if (focallength < 0) {
		throw std::runtime_error("focal length not set");
	}
	if ((star.x == 0) && (star.y == 0)) {
		throw std::runtime_error("calibration star not set");
	}
	guider->startCalibration(focallength);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the guide command
 */
int	guide_command() {
	throw std::runtime_error("guide command not implemented");
	return EXIT_SUCCESS;
}

/**
 * \brief Tracks command implementation
 *
 * The tracks command displays a list of tracks available. If the verbose
 * flag is set, then information about each track is also returned, i.e.
 * the number of points and the duration. This information requires that
 * the points be retrieved from the server as well. This is a little wasteful,
 * but the data size is still quite managable, and there does not seem to
 * be a performance issue from this.
 */
int	tracks_command(GuiderFactoryPrx guiderfactory,
		GuiderDescriptor descriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get tracks from remote server");
	idlist	l = guiderfactory->getGuideruns(descriptor);
	std::cout << l.size() << " tracks" << std::endl;
	for (auto ptr = l.begin(); ptr != l.end(); ptr++) {
		int	id = *ptr;
		if (verbose) {
			std::cout << astro::stringprintf("%4d: ", id);
			TrackingHistory	history
				= guiderfactory->getTrackingHistory(id);
			std::cout << astro::timeformat("%Y-%m-%d %H:%M",
				converttime((double)history.timeago));
			if (history.points.size() > 1) {
				std::cout << astro::stringprintf(" %6d pts",
					history.points.size());
				std::cout << astro::stringprintf("  %6.0fsec", 
					history.points.begin()->timeago
					- history.points.rbegin()->timeago);
			}
		} else {	
			std::cout << id;
		}
		std::cout << std::endl;
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Tracking point display class
 *
 * This class is a functor class used to display a tracking point. It also
 * keeps track of the index of the point being displayed.
 */
class TrackingPoint_display {
	std::ostream&	_out;
	int	counter;
public:
	TrackingPoint_display(std::ostream& out) : _out(out) {
		counter = 1;
	}
	void	operator()(const TrackingPoint& point);
};

void	TrackingPoint_display::operator()(const TrackingPoint& point) {
	_out << astro::stringprintf("[%04d] ", ++counter);
	_out << astro::timeformat("%Y-%m-%d %H:%M:%S",
		converttime(point.timeago));
	_out << astro::stringprintf(".%03.0f ",
		1000 * (point.timeago - trunc(point.timeago)));
	_out << astro::stringprintf("(%f,%f) -> (%f,%f)",
			point.trackingoffset.x, point.trackingoffset.y,
			point.activation.x, point.activation.y);
	_out << std::endl;
}

/**
 * \brief Implementation of the history command
 *
 * The tracking history is identified by the id. If the verbose flag is set,
 * then all the points of the tracking history are displayed.
 */
int	history_command(GuiderFactoryPrx guiderfactory, long id) {
	TrackingHistory	history = guiderfactory->getTrackingHistory(id);
	std::cout << history.guiderunid << ": ";
	std::cout << astro::timeformat("%Y-%m-%d %H:%M",
		converttime((double)history.timeago));
	std::cout << std::endl;
	if (verbose) {
		std::for_each(history.points.begin(), history.points.end(),
			TrackingPoint_display(std::cout)
		);
	}

	return EXIT_SUCCESS;
}

/**
 * \brief long options for the snow guiding program
 */
static struct option	longopts[] = {
{ "binning",		required_argument,	NULL,	'b' }, /*  0 */
{ "config",		required_argument,	NULL,	'c' }, /*  1 */
{ "debug",		no_argument,		NULL,	'd' }, /*  2 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  3 */
{ "help",		no_argument,		NULL,	'h' }, /*  4 */
{ "instrument",		required_argument,	NULL,	'i' }, /*  5 */
{ "rectangle",		required_argument,	NULL,	'r' }, /*  6 */
{ "prefix",		required_argument,	NULL,	'p' }, /*  7 */
{ "star",		required_argument,	NULL,	's' }, /*  8 */
{ "temperature",	required_argument,	NULL,	't' }, /*  9 */
{ "verbose",		no_argument,		NULL,	'v' }, /* 10 */
{ NULL,			0,			NULL,    0  }  /* 11 */
};

/**
 * \brief Main program for the snowguide program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);

	std::string	instrumentname;
	double	temperature = std::numeric_limits<double>::quiet_NaN();
	std::string	binning;
	std::string	frame;

	int	c;
	int	longindex;

	exposure.exposuretime = 1.;

	while (EOF != (c = getopt_long(argc, argv, "b:c:de:f:hi:r:s:t:v",
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
			exposure.exposuretime = std::stod(optarg);
			break;
		case 'f':
			focallength = std::stod(optarg);
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			instrumentname = optarg;
			break;
		case 'p':
			prefix = std::string(optarg);
			break;
		case 'r':
			frame = optarg;
			break;
		case 's':
			star = convert(astro::image::ImagePoint(optarg));
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		case 'v':
			verbose = true;
			break;
		}

	// next argument is the command
	if (argc <= optind) {
		throw std::runtime_error("missing command argument");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", command.c_str());

	// handle the help command
	if (command == "help") {
		return help_command();
	}

	// get the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
		throw std::runtime_error("instrument name not set");
	}
	RemoteInstrument	instrument(config->database(),
						instrumentname);

	// server of the camera
	astro::ServerName	servername(instrument.servername(
					astro::DeviceName::Camera));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument on server %s",
		std::string(servername).c_str());

	// get camera, ccd and proxy
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the device from the instrument");
	CameraPrx	camera = instrument.camera_proxy();
	CcdPrx		ccd = instrument.ccd_proxy();
	GuiderPortPrx	guiderport = instrument.guiderport_proxy();

	// build the guider descriptor
	GuiderDescriptor	descriptor;
	descriptor.cameraname = camera->getName();
	descriptor.ccdid = ccd->getInfo().id;
	descriptor.guiderportname = guiderport->getName();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera: %s",
		descriptor.cameraname.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd: %d", descriptor.ccdid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port: %s",
		descriptor.guiderportname.c_str());

	// connect to the guider factory of a remote server
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Guiders"));
	GuiderFactoryPrx	guiderfactory
		= GuiderFactoryPrx::checkedCast(base);

	// the next action depends on the command to execute. This first
	// group of commands does not need a guider
	if (command == "list") {
		return list_command(guiderfactory, descriptor);
	}
	if (command == "tracks") {
		return tracks_command(guiderfactory, descriptor);
	}
	if (command == "history") {
		if (argc <= optind) {
			throw std::runtime_error("missing history id");
		}
		long	historyid = std::stoi(argv[optind++]);
		return history_command(guiderfactory, historyid);
	}

	// retrieve a guider
	GuiderPrx	guider = guiderfactory->get(descriptor);
	GuiderState	state = guider->getState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found the guider in state %s",
		guiderstate2string(state).c_str());

	// commands needing a guider
	if (command == "cancel") {
		return cancel_command(guider);
	}
	if (command == "image") {
		if (argc <= optind) {
			throw std::runtime_error("");
		}
		return images_command(guider, std::string(argv[optind++]));
	}
	if (command == "monitor") {
		return monitor_command(guider);
	}
	if (command == "calibrate") {
		// next argument must be the calibration id, if it is present
		if (argc <= optind) {
			throw std::runtime_error("no calibration id specified");
		}
		int calibrationid = std::stoi(argv[optind++]);
		return calibrate_command(guider, calibrationid);
	}

	// the guide and calibrate commands need an exposure
	exposure.gain = 1;
	exposure.limit = 0;
	exposure.shutter = ShOPEN;
	exposure.purpose = ExLIGHT;
	if (binning.size() > 0) {
		exposure.mode = convert(astro::camera::Binning(binning));
	} else {
		exposure.mode.x = 1;
		exposure.mode.y = 1;
	}
	guider->setExposure(exposure);

	// implement the guide and calibrate commands
	if (command == "guide") {
		return guide_command();
	}
	if (command == "calibrate") {
		// next argument must be the calibration id, if it is present
		return calibrate_command(guider, -1);
	}

	throw std::runtime_error("unknown command");
}

} // namespace snowguide
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowguide::main>(argc, argv);
}
