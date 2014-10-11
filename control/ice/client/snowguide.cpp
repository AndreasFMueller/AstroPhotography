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

using namespace snowstar;

namespace snowguide {

/**
 * \brief usage method
 */
void	usage(const char *progname) {
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << p << " [ options ] help" << std::endl;
	std::cout << p << " [ options ] calibrate" << std::endl;
	std::cout << p << " [ options ] monitor" << std::endl;
	std::cout << p << " [ options ] guide [ <calibrationid> ] " << std::endl;
	std::cout << p << " [ options ] cancel" << std::endl;
	std::cout << p << " [ options ] list" << std::endl;
	std::cout << p << " [ options ] history" << std::endl;
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
 * \brief Implementation of the list command
 */
int	list_command(GuiderFactoryPrx guiderfactory,
		GuiderDescriptor descriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get calibrations from remote server");
	idlist	l = guiderfactory->getCalibrations(descriptor);
	std::cout << "number of calibrations: " << l.size() << std::endl;
	std::for_each(l.begin(), l.end(),
		[](int i) {
			std::cout << i << std::endl;
		}
	);
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
 * \brief Implementation of the history command
 */
int	history_command() {
	throw std::runtime_error("history command not implemented");
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
{ "temperature",	required_argument,	NULL,	't' }, /*  7 */
{ NULL,			0,			NULL,    0  }
};

/**
 * \brief Main program for the snowguide program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);

	std::string	instrumentname;
	double	exposuretime = 1.0;
	double	temperature = std::numeric_limits<double>::quiet_NaN();
	std::string	binning;
	std::string	frame;

	int	c;
	int	longindex;

	while (EOF != (c = getopt_long(argc, argv, "b:c:de:hi:r:t:",
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
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			instrumentname = optarg;
			break;
		case 'r':
			frame = optarg;
			break;
		case 't':
			temperature = std::stod(optarg);
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
	std::string     connectstring
                = astro::stringprintf("Guiders:default -h %s -p %hu",
                        servername.host().c_str(), servername.port());
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(connectstring);
	GuiderFactoryPrx	guiderfactory
		= GuiderFactoryPrx::checkedCast(base);

	// retrieve a guider
	GuiderPrx	guider = guiderfactory->get(descriptor);
	GuiderState	state = guider->getState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found the guider in state %s",
		guiderstate2string(state).c_str());

	// the next action depends on the command to execute
	if (command == "cancel") {
		return cancel_command(guider);
	}
	if (command == "guide") {
		return guide_command();
	}
	if (command == "calibrate") {
		// next argument must be the calibration id, if it is present
		int	calibrationid = -1;
		if (argc > optind) {
			calibrationid = std::stoi(argv[optind++]);
		}
		return calibrate_command(guider, calibrationid);
	}
	if (command == "list") {
		return list_command(guiderfactory, descriptor);
	}
	if (command == "history") {
		return history_command();
	}

	return EXIT_SUCCESS;
}

} // namespace snowguide

int	main(int argc, char *argv[]) {
	return astro::main_function<snowguide::main>(argc, argv);
}
