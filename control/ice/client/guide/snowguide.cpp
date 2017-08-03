/*
 * snowguide.cpp -- command line client to control guiding
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hohschule Rapperswil
 */
#include <AstroUtils.h>
#include <iostream>
#include <cstdlib>
#include <typeinfo>
#include <getopt.h>
#include <CommunicatorSingleton.h>
#include <AstroConfig.h>
#include <IceConversions.h>
#include "guide.h"

using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowguide {

/**
 * \brief long options for the snow guiding program
 */
static struct option	longopts[] = {
{ "adaptiveoptics",	required_argument,	NULL,	'a' }, /*  0 */
{ "aointerval",		required_argument,	NULL,	'A' }, /*  1 */
{ "binning",		required_argument,	NULL,	'b' }, /*  2 */
{ "ccd",		required_argument,	NULL,	'C' }, /*  3 */
{ "config",		required_argument,	NULL,	'c' }, /*  4 */
{ "csv",		no_argument,		NULL,	 1  }, /*  5 */
{ "debug",		no_argument,		NULL,	'd' }, /*  6 */
{ "dark",		no_argument,		NULL,	'D' }, /*  7 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  8 */
{ "flipped",		no_argument,		NULL,	'f' }, /*  9 */
{ "guideport",		required_argument,	NULL,	'G' }, /* 10 */
{ "help",		no_argument,		NULL,	'h' }, /* 11 */
{ "interval",		required_argument,	NULL,	'i' }, /* 12 */
{ "imagecount",		required_argument,	NULL,	'I' }, /* 13 */
{ "limit",		required_argument,	NULL,	'l' }, /* 14 */
{ "method",		required_argument,	NULL,	'm' }, /* 15 */
{ "prefix",		required_argument,	NULL,	'p' }, /* 16 */
{ "rectangle",		required_argument,	NULL,	'r' }, /* 17 */
{ "star",		required_argument,	NULL,	's' }, /* 18 */
{ "stepping",		no_argument,		NULL,	'S' }, /* 19 */
{ "temperature",	required_argument,	NULL,	't' }, /* 20 */
{ "verbose",		no_argument,		NULL,	'v' }, /* 21 */
{ "width",		required_argument,	NULL,	'w' }, /* 22 */
{ NULL,			0,			NULL,    0  }  /* 23 */
};

/**
 * \brief Main program for the snowguide program
 */
int	main(int argc, char *argv[]) {
	CommunicatorSingleton	communicator(argc, argv);

	double	temperature = std::numeric_limits<double>::quiet_NaN();
	std::string	binning;
	std::string	frame;
	Guide	guide;

	int	c;
	int	longindex;
	int	ccdIndex = 0;
	int	guideportIndex = 0;
	int	adaptiveopticsIndex = 0;
	int	width = -1;

	guide.exposure.exposuretime = 1.;

	while (EOF != (c = getopt_long(argc, argv,
		"A:a:b:c:C:de:f:G:hi:m:r:s:t:vw:", longopts, &longindex)))
		switch (c) {
		case 'a':
			adaptiveopticsIndex = std::stoi(optarg);
			break;
		case 'A':
			guide.aointerval = std::stod(optarg);
			break;
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
		case 'D':
			guide.usedark = true;
			break;
		case 'e':
			guide.exposure.exposuretime = std::stod(optarg);
			break;
		case 'f':
			guide.flipped = true;
			break;
		case 'G':
			guideportIndex = std::stoi(optarg);
			break;
		case 'h':
			guide.usage(argv[0]);
			return EXIT_SUCCESS;
		case 'i':
			guide.guideinterval = std::stod(optarg);
			break;
		case 'l':
			guide.badpixellimit = std::stod(optarg);
			break;
		case 'm': {
			std::string	m(optarg);
			if (m == "null") {
				guide.method = TrackerNULL;
			} else if (m == "star") {
				guide.method = TrackerSTAR;
			} else if (m == "phase") {
				guide.method = TrackerPHASE;
			} else if (m == "diff") {
				guide.method = TrackerDIFFPHASE;
			} else if (m == "laplace") {
				guide.method = TrackerLAPLACE;
			} else if (m == "large") {
				guide.method = TrackerLARGE;
			} else {
				std::string	msg = astro::stringprintf(
					"unkown tracker method: %s", m.c_str());
				debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
				throw std::runtime_error(msg);
			}
			}
			break;
		case 'p':
			guide.prefix = std::string(optarg);
			break;
		case 'r':
			frame = optarg;
			break;
		case 's':
			guide.star = convert(astro::image::ImagePoint(optarg));
			break;
		case 'S':
			guide.stepping = true;
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		case 'v':
			guide.verbose = true;
			break;
		case 'w':
			width = std::stoi(optarg);
			break;
		case 1:
			guide.csv = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// next argument is the command
	if (argc <= optind) {
		throw std::runtime_error("missing  argument");
		guide.usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	argument(argv[optind++]);
	
	// handle simple help argument
	if ("help" == argument) {
		return guide.help_command(argv[0]);
	}

	// if this is not the help command, then we need three more
	// arguments
	astro::ServerName	servername(argument);
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
		return guide.help_command(argv[0]);
	}

	// server of the instrument
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument %s on server %s",
		std::string(instrumentname).c_str(),
		servername.toString().c_str());

	// build the guider descriptor
	GuiderDescriptor	descriptor;
	descriptor.instrumentname = instrumentname;
	descriptor.ccdIndex = ccdIndex;
	descriptor.guideportIndex = guideportIndex;
	descriptor.adaptiveopticsIndex = adaptiveopticsIndex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument: %s",
		descriptor.instrumentname.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd: %d", descriptor.ccdIndex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guideport: %d",
		descriptor.guideportIndex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adaptiveoptics: %d",
		descriptor.adaptiveopticsIndex);

	// connect to the guider factory of a remote server
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();
        Ice::ObjectPrx  gbase
		= ic->stringToProxy(servername.connect("Guiders"));
	GuiderFactoryPrx	guiderfactory
		= GuiderFactoryPrx::checkedCast(gbase);

	// the next action depends on the command to execute. This first
	// group of commands does not need a guider
	if (command == "list") {
		return guide.list_command(guiderfactory, descriptor);
	}
	if (command == "tracks") {
		return guide.tracks_command(guiderfactory, descriptor);
	}
	if (command == "forget") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(std::stoi(argv[optind++]));
		}
		return guide.forget_command(guiderfactory, ids);
	}
	if (command == "history") {
		if (argc <= optind) {
			throw std::runtime_error("missing history id");
		}
		long	historyid = std::stoi(argv[optind++]);
		if (argc > optind) {
			ControlType	type = ControlGuidePort;
			std::string	typestring(argv[optind++]);
			try {
				type = Guide::string2type(typestring);
			} catch (...) { }
			return guide.history_command(guiderfactory,
				historyid, type);
		}
		return guide.history_command(guiderfactory, historyid);
	}
	if (command == "trash") {
		std::list<int>	ids;
		while (optind < argc) {
			ids.push_back(std::stoi(argv[optind++]));
		}
		return guide.trash_command(guiderfactory, ids);
	}

	// retrieve a guider
	GuiderPrx	guider = guiderfactory->get(descriptor);
	GuiderState	state = guider->getState();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found the guider in state %s",
		guiderstate2string(state).c_str());

	// commands needing a guider
	if (command == "state") {
		return guide.state_command(guider);
	}
	if (command == "stop") {
		return guide.stop_command(guider);
	}
	if (command == "dark") {
		return guide.dark_command(guider);
	}
	if (command == "flat") {
		return guide.flat_command(guider);
	}
	if (command == "image") {
		if (argc < optind) {
			throw std::runtime_error("missing filename argument");
		}
		std::string	filename(argv[optind++]);
		return guide.image_command(guider, filename);
	}
	if (command == "darkimage") {
		if (argc < optind) {
			throw std::runtime_error("missing filename argument");
		}
		std::string	filename(argv[optind++]);
		return guide.darkimage_command(guider, filename);
	}
	if (command == "flatimage") {
		if (argc < optind) {
			throw std::runtime_error("missing filename argument");
		}
		std::string	filename(argv[optind++]);
		return guide.flatimage_command(guider, filename);
	}
	if (command == "repository") {
		if (argc > optind) {
			try {
				std::string	reponame(argv[optind++]);
				debug(LOG_DEBUG, DEBUG_LOG, 0, "repo name: %s",
					reponame.c_str());
				return guide.repository_command(guider, reponame);
			} catch (const NotFound& x) {
				std::cerr << x.cause << std::endl;
				return EXIT_FAILURE;
			}
		} else {
			return guide.repository_command(guider);
		}
	}
	if (command == "calibration") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "argc = %d, optind = %d",
			argc, optind);
		if (argc > optind) {
			return guide.calibration_command(guiderfactory,
				guider, std::string(argv[optind++]));
		}
		return guide.calibration_command(guiderfactory, guider);
	}
	if (command == "cancel") {
		return guide.cancel_command(guider);
	}
	if ((command == "image") || (command == "images")) {
		if (argc <= optind) {
			throw std::runtime_error("");
		}
		return guide.images_command(guider,
			std::string(argv[optind++]));
	}
	if (command == "monitor") {
		return guide.monitor_command(guider);
	}
	if (command == "uncalibrate") {
		if (argc <= optind) {
			throw std::runtime_error("missing type argument");
		}
		std::string	type(argv[optind++]);
		return guide.uncalibrate_command(guider,
			Guide::string2type(type));
	}
	if (command == "flip") {
		if (argc <= optind) {
			return guide.flip_command(guider);
		}
		std::string	type(argv[optind++]);
		return guide.flip_command(guider,
			Guide::string2type(type));
	}

	// the guide and calibrate commands need an exposure
	guide.exposure.gain = 1;
	guide.exposure.limit = 0;
	guide.exposure.shutter = ShOPEN;
	guide.exposure.purpose = ExGUIDE;
	if (binning.size() > 0) {
		guide.exposure.mode = convert(astro::image::Binning(binning));
	} else {
		guide.exposure.mode.x = 1;
		guide.exposure.mode.y = 1;
	}
	if (frame.size() > 0) {
		guide.exposure.frame = convert(
			astro::image::ImageRectangle(frame));
	} else {
		guide.exposure.frame.origin.x = guide.star.x - width / 2;
		guide.exposure.frame.origin.y = guide.star.y - width / 2;
		guide.exposure.frame.size.width = width;
		guide.exposure.frame.size.height = width;
	}
	guider->setExposure(guide.exposure);

	// make sure we have the guide star set
	Point	starpoint;
	starpoint.x = guide.star.x;
	starpoint.y = guide.star.y;
	guider->setStar(starpoint);

	// implement the guide and calibrate commands
	if (command == "guide") {
		return guide.guide_command(guider);
	}
	if (command == "calibrate") {
		// next argument must be the calibration id, if it is present
		if (argc > optind) {
			return guide.calibrate_command(guider,
				std::string(argv[optind++]));
		}
		return guide.calibrate_command(guider, -1);
	}

	std::string	msg = astro::stringprintf("unknown command '%s'",
		command.c_str());
	guide.usage(argv[0]);
	throw std::runtime_error(msg);
}

} // namespace snowguide
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	return astro::main_function<snowstar::app::snowguide::main>(argc, argv);
}
