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
Exposure	exposure;
std::string	prefix("p");
volatile bool	completed = false;
float	guideinterval = 10;
bool	csv = false;

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
	std::cout << p << " [ options ] <service> <INSTRUMENT> help"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> calibrate"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> calibration"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> trash <calid>"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> state"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> stop"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> monitor"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> images"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> guide"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> cancel"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> list"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> tracks"
		<< std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> history"
		" [ trackid ]" << std::endl;
	std::cout << p << " [ options ] <service> <INSTRUMENT> forget <trackid> ...";
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
	std::cout << " -h,--help             display this help message and exit";
	std::cout << std::endl;
	std::cout << " -i,--interval=<i>     perform an update ever i seconds when guiding";
	std::cout << std::endl;
	std::cout << " -r,--rectangle=<rec>  expose only a subrectangle as "
		"specified by <rec>." << std::endl;
	std::cout << "                       <rec> must be of the form";
	std::cout << std::endl;
	std::cout << "                       widthxheight@(xoffset,yoffset)";
	std::cout << std::endl;
	std::cout << "                       if -s and -w are specified, the "
		"subrectangle is";
	std::cout << std::endl;
	std::cout << "                       computed from these." << std::endl;
	std::cout << " -s,--star=<pos>       position of the star to calibrate "
		"or guide on in the" << std::endl;
	std::cout << "                       syntax (x,y), the parentheses are "
		"optional" << std::endl;
	std::cout << " -t,--temperature=<t>  cool ccd to temperature <t>, "
		"ignored if the instrument" << std::endl;
	std::cout << "                       has no cooler" << std::endl;
	std::cout << " -v,--verbose          enable verbose mode" << std::endl;
	std::cout << " -w,--width=<w>        set the width and height of the area to expose" << std::endl;
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
"    run. In the latter case a star to perform the calibration" << std::endl <<
"    on must be specified with the -s option." << std::endl
<< std::endl << 
"calibration" << std::endl <<
"    display the current calibration"
<< std::endl << 
"monitor" << std::endl <<
"    Monitor the guiding process. This subcommand reports all" << std::endl <<
"    state changes and all commands sent to the telescope mount" << std::endl
<< std::endl << 
"guide" << std::endl <<
"    Start guiding with the current calibration id." << std::endl <<
"    the --star option is required." << std::endl << 
"stop" << std::endl <<
"    stop the guiding process" << std::endl
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
"tracks"  << std::endl <<
"    list all guiding tracks recorded in the database" << std::endl
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
	_out << cal.points.size() << " points, ";
	_out << astro::stringprintf("quality=%.1f%%, ", 100 * cal.quality);
	_out << astro::stringprintf("%.3f mas/Pixel", cal.masPerPixel);
	_out << std::endl;

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
 * \brief Tracking point display class
 *
 * This class is a functor class used to display a tracking point. It also
 * keeps track of the index of the point being displayed.
 */
class TrackingPoint_display {
	std::ostream&	_out;
	int	counter;
	double	_starttime;
	bool	_csv;
public:
	bool	csv() const { return _csv; }
	void	csv(bool c) { _csv = c; }
private:
	double	_masperpixel;
public:
	double	masperpixel() const { return _masperpixel; }
	void	masperpixel(double m) { _masperpixel = m; }
public:
	TrackingPoint_display(std::ostream& out, double starttime)
		: _out(out), _starttime(starttime) {
		counter = 1;
	}
	void	operator()(const TrackingPoint& point);
};

void	TrackingPoint_display::operator()(const TrackingPoint& point) {
	if (_csv) {
		_out << astro::stringprintf("%6d,", ++counter);
		_out << astro::stringprintf("%8.1f,",
			_starttime - point.timeago);
		_out << astro::stringprintf("%10.4f,%10.4f,%10.4f,%10.4f",
				point.trackingoffset.x, point.trackingoffset.y,
				point.activation.x, point.activation.y);
		double	p = hypot(point.trackingoffset.x,
			point.trackingoffset.y) * masperpixel();
		_out << astro::stringprintf(",%8.0f", p);
	} else {
		_out << astro::stringprintf("[%04d] ", ++counter);
		_out << astro::timeformat("%Y-%m-%d %H:%M:%S",
			converttime(point.timeago));
		_out << astro::stringprintf(".%03.0f ",
			1000 * (point.timeago - trunc(point.timeago)));
		_out << astro::stringprintf("(%f,%f) -> (%f,%f)",
				point.trackingoffset.x, point.trackingoffset.y,
				point.activation.x, point.activation.y);
	}
	_out << std::endl;
}


/**
 * \brief Get the state of a guider
 *
 * This command retrieves the current state of the guider
 */
int	state_command(GuiderPrx guider) {
	GuiderState	state = guider->getState();
	std::cout << guiderstate2string(state);
	switch (state) {
	case GuiderCALIBRATING:
		std::cout << guider->calibrationProgress();
		break;
	default:
		break;
	}
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Stop the guider 
 */
int	stop_command(GuiderPrx guider) {
	GuiderState	state = guider->getState();
	if (state != GuiderGUIDING) {
		std::cerr << "not guiding" << std::endl;
		return EXIT_FAILURE;
	}
	guider->stopGuiding();
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
 * \brief Common infrastructure for monitor classes
 */
class CommonMonitor {
	std::mutex	mtx;
	std::condition_variable	cond;
	bool	_complete;
public:
	bool	complete() const { return _complete; }
	void	complete(bool c) {
		_complete = c;
		if (_complete) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibraition complete");
			std::unique_lock<std::mutex>	lock(mtx);
			cond.notify_one();
		}
	}
	CommonMonitor() : _complete(false) {
	}
	void	wait() {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for completion");
		std::unique_lock<std::mutex>	lock(mtx);
		cond.wait(lock);
	}
};

/**
 * \brief Calibration monitor class
 */
class CalibrationMonitorI : public CalibrationMonitor, public CommonMonitor {
	CalibrationPoint_display	display;
public:
	CalibrationMonitorI() : display(std::cout) {
	}
	void	update(const CalibrationPoint& point,
		const Ice::Current& /* current */) {
		display(point);
	}
	void	stop(const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
		complete(true);
	}
};

int	monitor_calibration(GuiderPrx guider) {
	debug(LOG_INFO, DEBUG_LOG, 0, "monitoring calibration");
	// create a new calibration monitor
	CalibrationMonitorI	*monitor = new CalibrationMonitorI();

	// register the monitor with the guider server
	Ice::ObjectPtr	callback = monitor;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	guider->ice_getConnection()->setAdapter(adapter.adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register calibration monitor");
	guider->registerCalibrationMonitor(ident);

	// wait for termination of the monitor
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for calibration completion");
	monitor->wait();

	// unregister the monitor
	guider->unregisterCalibrationMonitor(ident);
	return EXIT_SUCCESS;
}

class TrackingMonitorI : public TrackingMonitor, public CommonMonitor {
	TrackingPoint_display	display;
public:
	TrackingMonitorI() : display(std::cout, 0) {
	}
	void	update(const TrackingPoint& point,
		const Ice::Current& /* current */) {
		display(point);
	}
	void	stop(const Ice::Current& /* current */) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stop received");
		complete(true);
	}
};

int	monitor_guiding(GuiderPrx guider) {
	debug(LOG_INFO, DEBUG_LOG, 0, "monitoring guiding");
	// create a new calibration monitor
	TrackingMonitorI	*monitor = new TrackingMonitorI();

	// register the monitor with the guider server
	Ice::ObjectPtr	callback = monitor;
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
	CallbackAdapter	adapter(ic);
	Ice::Identity	ident = adapter.add(callback);
	guider->ice_getConnection()->setAdapter(adapter.adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register tracking monitor");
	guider->registerTrackingMonitor(ident);

	// wait for termination of the monitor
	debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for guiding completion");
	monitor->wait();

	// unregister the monitor
	guider->unregisterTrackingMonitor(ident);
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the monitor command
 */
int	monitor_command(GuiderPrx guider) {
	// The type of callback to install depends on the current guider state
	switch (guider->getState()) {
	case GuiderUNCONFIGURED:
	case GuiderIDLE:
	case GuiderCALIBRATED:
		std::cout << "not in monitorable state" << std::endl;
	case GuiderCALIBRATING:
		return monitor_calibration(guider);
	case GuiderGUIDING:
		return monitor_guiding(guider);
	}
	return EXIT_FAILURE;
}

/**
 * \brief Display calibration
 *
 * This command retrieves the calibration information from the guider
 * and displays it
 */
int	calibration_command(GuiderFactoryPrx guiderfactory, GuiderPrx guider,
		int calibrationid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving calibration %d",
		calibrationid);
	Calibration	cal;
	if (calibrationid < 0) {
		switch (guider->getState()) {
		case GuiderUNCONFIGURED:
		case GuiderIDLE:
		case GuiderCALIBRATING:
			std::cerr << "not calibrated, specify calibration id";
			std::cerr << std::endl;
			return EXIT_FAILURE;
			break;
		case GuiderCALIBRATED:
		case GuiderGUIDING:
			cal = guider->getCalibration();
			break;
		}
	} else {
		cal = guiderfactory->getCalibration(calibrationid);
	}

	Calibration_display	cd(std::cout);
	cd(cal);
	std::cout << std::endl;
	return EXIT_SUCCESS;
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
 * \brief Remove calibrations
 */
int	trash_command(GuiderFactoryPrx guiderfactory, std::list<int> ids) {
	std::list<int>::const_iterator	i;
	for (i = ids.begin(); i != ids.end(); i++) {
		try {
			guiderfactory->deleteCalibration(*i);
		} catch (const NotFound& x) {
			std::cerr << "cannot delete calibration " << *i << ": ";
			std::cerr << x.cause << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of calibrate command
 */
int	calibrate_command(GuiderPrx guider, int calibrationid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "use calibrationid = %d",
		calibrationid);
	if (calibrationid > 0) {
		guider->useCalibration(calibrationid);
		return EXIT_SUCCESS;
	} else {
		if ((star.x == 0) && (star.y == 0)) {
			throw std::runtime_error("calibration star not set");
		}
	}
	calibrationid = guider->startCalibration();
	std::cout << "new calibration " << calibrationid << " in progress";
	std::cout << std::endl;
	return EXIT_SUCCESS;
}

/**
 * \brief Implementation of the guide command
 */
int	guide_command(GuiderPrx guider) {
	if ((star.x == 0) && (star.y == 0)) {
		throw std::runtime_error("calibration star not set");
	}
	// make sure we have all the information we need
	if ((guideinterval < 0) || (guideinterval > 60)) {
		std::string	cause = astro::stringprintf(
			"bad guideinterval: %.3f", guideinterval);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw std::runtime_error(cause);
	}
	
	// get the guider
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding with interval %.1f",
		guideinterval);
	guider->startGuiding(guideinterval);

	// we are done
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
 * \brief Implementation of the history command
 *
 * The tracking history is identified by the id. If the verbose flag is set,
 * then all the points of the tracking history are displayed.
 */
int	history_command(GuiderFactoryPrx guiderfactory, long historyid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving history %d", historyid);
	TrackingHistory	history = guiderfactory->getTrackingHistory(historyid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "track uses calibration %d",
		history.calibrationid);
	if (csv) {
		std::cout << "number,    time,   xoffset,   yoffset,     xcorr,     ycorr,  offset" << std::endl;
		verbose = csv;
	} else {
		std::cout << history.guiderunid << ": ";
		std::cout << astro::timeformat("%Y-%m-%d %H:%M",
			converttime((double)history.timeago));
		std::cout << std::endl;
	}
	if (verbose) {
		Calibration	cal = guiderfactory->getCalibration(
					history.calibrationid);
		double	starttime = history.points.begin()->timeago;
		TrackingPoint_display	display(std::cout, starttime);
		display.csv(csv);
		display.masperpixel(cal.masPerPixel);
		std::for_each(history.points.begin(), history.points.end(),
			display);
	}

	return EXIT_SUCCESS;
}

/**
 * \brief Forget tracking histories
 */
int	forget_command(GuiderFactoryPrx guiderfactory,
		const std::list<int>& ids) {
	std::list<int>::const_iterator	i;
	for (i = ids.begin(); i != ids.end(); i++) {
		try {
			guiderfactory->deleteTrackingHistory(*i);
		} catch (const NotFound& x) {
			std::cerr << "cannot delete tracking history ";
			std::cerr << *i << ": ";
			std::cerr << x.cause << std::endl;
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

/**
 * \brief long options for the snow guiding program
 */
static struct option	longopts[] = {
{ "binning",		required_argument,	NULL,	'b' }, /*  0 */
{ "ccd",		required_argument,	NULL,	'C' }, /*  1 */
{ "config",		required_argument,	NULL,	'c' }, /*  2 */
{ "csv",		no_argument,		NULL,	 1  }, /*  3 */
{ "debug",		no_argument,		NULL,	'd' }, /*  4 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  5 */
{ "guiderport",		required_argument,	NULL,	'G' }, /*  7 */
{ "help",		no_argument,		NULL,	'h' }, /*  8 */
{ "interval",		required_argument,	NULL,	'i' }, /*  9 */
{ "prefix",		required_argument,	NULL,	'p' }, /* 10 */
{ "rectangle",		required_argument,	NULL,	'r' }, /* 11 */
{ "star",		required_argument,	NULL,	's' }, /* 12 */
{ "temperature",	required_argument,	NULL,	't' }, /* 13 */
{ "verbose",		no_argument,		NULL,	'v' }, /* 14 */
{ "width",		required_argument,	NULL,	'w' }, /* 15 */
{ NULL,			0,			NULL,    0  }  /* 16 */
};

/**
 * \brief Main program for the snowguide program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);

	double	temperature = std::numeric_limits<double>::quiet_NaN();
	std::string	binning;
	std::string	frame;

	int	c;
	int	longindex;
	int	ccdIndex = 0;
	int	guiderportIndex = 0;
	int	width = -1;

	exposure.exposuretime = 1.;

	while (EOF != (c = getopt_long(argc, argv, "b:c:C:de:f:G:hi:r:s:t:vw:",
		longopts, &longindex)))
		switch (c) {
		case 'b':
			binning = optarg;
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'C':
			ccdIndex = std::stoi(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposure.exposuretime = std::stod(optarg);
			break;
		case 'G':
			guiderportIndex = std::stoi(optarg);
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			guideinterval = std::stod(optarg);
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
		case 'w':
			width = std::stoi(optarg);
			break;
		case 1:
			csv = true;
			break;
		}

	// next argument is the command
	if (argc <= optind) {
		throw std::runtime_error("missing service argument");
	}
	astro::ServerName	servername(argv[optind++]);
	if (argc <= optind) {
		throw std::runtime_error("missing instrument name argument");
	}
	std::string	instrumentname(argv[optind++]);
	if (argc <= optind) {
		throw std::runtime_error("missing command argument");
	}
	std::string	command = argv[optind++];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", command.c_str());

	// handle the help command
	if (command == "help") {
		return help_command();
	}

#if 0
	// get the configuration
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
#endif

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
		throw std::runtime_error("instrument name not set");
	}

	// server of the instrument
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument on server %s",
		std::string(instrumentname).c_str());

	// build the guider descriptor
	GuiderDescriptor	descriptor;
	descriptor.instrumentname = instrumentname;
	descriptor.ccdIndex = ccdIndex;
	descriptor.guiderportIndex = guiderportIndex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument: %s",
		descriptor.instrumentname.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd: %d", descriptor.ccdIndex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port: %d",
		descriptor.guiderportIndex);

	// connect to the guider factory of a remote server
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  gbase
		= ic->stringToProxy(servername.connect("Guiders"));
	GuiderFactoryPrx	guiderfactory
		= GuiderFactoryPrx::checkedCast(gbase);

	// the next action depends on the command to execute. This first
	// group of commands does not need a guider
	if (command == "list") {
		return list_command(guiderfactory, descriptor);
	}
	if (command == "tracks") {
		return tracks_command(guiderfactory, descriptor);
	}
	if (command == "forget") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(std::stoi(argv[optind++]));
		}
		return forget_command(guiderfactory, ids);
	}
	if (command == "history") {
		if (argc <= optind) {
			throw std::runtime_error("missing history id");
		}
		long	historyid = std::stoi(argv[optind++]);
		return history_command(guiderfactory, historyid);
	}
	if (command == "trash") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(std::stoi(argv[optind++]));
		}
		return trash_command(guiderfactory, ids);
	}

	// retrieve a guider
	GuiderPrx	guider = guiderfactory->get(descriptor);
	GuiderState	state = guider->getState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found the guider in state %s",
		guiderstate2string(state).c_str());

	// commands needing a guider
	if (command == "state") {
		return state_command(guider);
	}
	if (command == "stop") {
		return stop_command(guider);
	}
	if (command == "calibration") {
		int	calibrationid = -1;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "argc = %d, optind = %d",
			argc, optind);
		if (argc > optind) {
			calibrationid = std::stoi(argv[optind++]);
		}
		return calibration_command(guiderfactory, guider, calibrationid);
	}
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

	// the guide and calibrate commands need an exposure
	exposure.gain = 1;
	exposure.limit = 0;
	exposure.shutter = ShOPEN;
	exposure.purpose = ExGUIDE;
	if (binning.size() > 0) {
		exposure.mode = convert(astro::camera::Binning(binning));
	} else {
		exposure.mode.x = 1;
		exposure.mode.y = 1;
	}
	if (frame.size() > 0) {
		exposure.frame = convert(astro::image::ImageRectangle(frame));
	} else {
		exposure.frame.origin.x = star.x - width / 2;
		exposure.frame.origin.y = star.y - width / 2;
		exposure.frame.size.width = width;
		exposure.frame.size.height = width;
	}
	guider->setExposure(exposure);

	// make sure we have the guide star set
	Point	starpoint;
	starpoint.x = star.x;
	starpoint.y = star.y;
	guider->setStar(starpoint);

	// implement the guide and calibrate commands
	if (command == "guide") {
		return guide_command(guider);
	}
	if (command == "calibrate") {
		// next argument must be the calibration id, if it is present
		int	calibrationid = -1;
		if (argc > optind) {
			calibrationid = std::stoi(argv[optind++]);
		}
		return calibrate_command(guider, calibrationid);
	}

	std::string	msg = astro::stringprintf("unknown command '%s'",
		command.c_str());
	throw std::runtime_error(msg);
}

} // namespace snowguide
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowguide::main>(argc, argv);
}
