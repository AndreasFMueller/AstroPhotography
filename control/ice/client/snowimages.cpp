/*
 * snowimages.cpp -- tool to retrieve a sequence of images from a remote camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>
#include <AstroConfig.h>
#include <AstroProject.h>
#include <AstroIO.h>
#include <Ice/Ice.h>
#include <device.h>
#include <camera.h>
#include <CommunicatorSingleton.h>
#include <RemoteInstrument.h>
#include <IceConversions.h>
#include <CommonClientTasks.h>
#include <stdexcept>

using namespace astro;
using namespace astro::module;
using namespace astro::device;
using namespace astro::image;
using namespace astro::io;
using namespace astro::config;
using namespace astro::project;
using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowimages {

/**
 * \brief Short usage function for the snowimages program
 */
static void	short_usage(const char *progname) {
	std::cout << "Usage:" << std::endl;
	astro::Path	path(progname);
	std::string	p = std::string("    ") + path.basename();
	std::cout << p << " [ options ] <service> <INSTRUMENT>" << std::endl;
	std::cout << p << " --help     for more information" << std::endl;
}

/**
 * \brief Usage function for the snoimages program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage: " << path.basename() << " [ options ] <service> <INSTRUMENT>"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--binning=XxY      select XxY binning mode (default 1x1)"
		<< std::endl;
	std::cout << " -C,--ccd=<index>      use a different ccd index than 0";
	std::cout << std::endl;
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
	std::cout << " -F,--focus=<F>        move to focus position <F> before "
		"exposing, ignored";
	std::cout << std::endl;
	std::cout << "                       if th einstrument has no focuser";
	std::cout << std::endl;
	std::cout << " -h,--help             display this help message and exit";
	std::cout << std::endl;
	std::cout << " -n,--number=<n>       take <n> exposures with these "
			"settings";
	std::cout << std::endl;
	std::cout << " -p,--purpose=<p>      images have purpose <p>, i.e. one "
		"of light, dark, flat,";
	std::cout << std::endl;
	std::cout << "                       bias or test";
	std::cout << std::endl;
	std::cout << " -P,--project=<proj>   Project name to store with the "
		"image" << std::endl;
	std::cout << " --rectangle=<rec>     expose only a subrectangle as "
		"specified by <rec>.";
	std::cout << std::endl;
	std::cout << "                       <rec> must be of the form";
	std::cout << std::endl;
	std::cout << "                       widthxheight@(xoffset,yoffset)";
	std::cout << std::endl;
	std::cout << " -r,--repo=<repo>      write images to repository <repo>";
	std::cout << std::endl;
	std::cout << " -t,--temperature=<t>  cool ccd to temperature <t>, "
		"ignored if the instrument";
	std::cout << std::endl;
	std::cout << "                       has no cooler";
	std::cout << std::endl;
}

/**
 * \brief Long options for the snowimages program
 */
static struct option	longopts[] = {
/* name			argument?		int*	int */
{ "binning",		required_argument,	NULL,	'b' }, /*  0 */
{ "ccd",		required_argument,	NULL,	'C' }, /*  1 */
{ "config",		required_argument,	NULL,	'c' }, /*  2 */
{ "debug",		no_argument,		NULL,	'd' }, /*  3 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  4 */
{ "filter",		required_argument,	NULL,	'f' }, /*  5 */
{ "focus",		required_argument,	NULL,	'F' }, /*  6 */
{ "help",		no_argument,		NULL,	'h' }, /*  7 */
{ "number",		required_argument,	NULL,	'n' }, /*  8 */
{ "purpose",		required_argument,	NULL,	'p' }, /*  9 */
{ "project",		required_argument,	NULL,	'P' }, /* 10 */
{ "rectangle",		required_argument,	NULL,	 1  }, /* 11 */
{ "repo",		required_argument,	NULL,	'r' }, /* 12 */
{ "temperature",	required_argument,	NULL,	't' }, /* 13 */
{ NULL,			0,			NULL,    0  }
};

/**
 * \brief Main function for the snowimages program
 */
int	main(int argc, char *argv[]) {
	debug_set_ident("snowimages");
	snowstar::CommunicatorSingleton	cs(argc, argv);
	Ice::CommunicatorPtr	ic = CommunicatorSingleton::get();

	int	nImages = 1;
	float	exposuretime = 1.; // default exposure time: 1 second
	double	temperature = std::numeric_limits<double>::quiet_NaN(); 

	debugtimeprecision = 3;
	debugthreads = 1;
	std::string	binning;
	std::string	filtername;
	std::string	reponame;

	// exposure structure
	astro::image::ImageRectangle	frame;
	astro::camera::Exposure::purpose_t	purpose
		= astro::camera::Exposure::light;

	// project
	std::string	project;

	// focus position
	unsigned short	focusposition = 0;

	// ccd index
	int	ccd_index = 0;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:C:c:de:f:F:hn:p:P:r:t:",
		longopts, &longindex))) {
		switch (c) {
		case 'b':
			binning = optarg;
			break;
		case 'C':
			ccd_index = std::stoi(optarg);
			break;
		case 'c':
			astro::config::Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'f':
			filtername = optarg;
			break;
		case 'F':
			focusposition = std::stol(optarg);
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'n':
			nImages = atoi(optarg);
			break;
		case 'p':
			purpose = astro::camera::Exposure::string2purpose(optarg);
			break;
		case 'P':
			project = std::string(optarg);
			break;
		case 'r':
			reponame = optarg;
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		case 1:
			switch (longindex) {
			case 11:
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"rectangle: %s", optarg);
				frame = astro::image::ImageRectangle(optarg);
				break;
			default:
				debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring %d",
					longindex);
				break;
			}
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// server and instrument name
	if (optind >= argc) {
		short_usage(argv[0]);
		throw std::runtime_error("service name missing");
	}
	astro::ServerName	servername(argv[optind++]);
	if (optind >= argc) {
		short_usage(argv[0]);
		throw std::runtime_error("instrument name missing");
	}
	std::string	instrumentname(argv[optind++]);

	// get the configuration
	astro::config::ConfigurationPtr	config = astro::config::Configuration::get();

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
		short_usage(argv[0]);
		throw std::runtime_error("instrument name not set");
	}
	Ice::ObjectPrx  base = ic->stringToProxy(
				servername.connect("Instruments"));
	InstrumentsPrx  instruments = InstrumentsPrx::checkedCast(base);
	//InstrumentPrx	instrument = instruments->get(instrumentname);

	// make sure we have a repository, because we would not know
	// where to store the images otherwise
	if (0 == reponame.size()) {
		// try to get the default repo name from the config database
		if (config->has("global", "repository", "default")) {
			reponame = config->get("global", "repository", "default");
		} else {
			short_usage(argv[0]);
			throw std::runtime_error("repository name not set");
		}
	}
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);

	// Create a remote instrument
	RemoteInstrument	ri(instruments, instrumentname);

	// get the Ccd
	snowstar::CcdPrx	ccd = ri.ccd(ccd_index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd");

	CcdTask	ccdtask(ccd);

	// if the focuser is specified, we try to get it and then set
	// the focus value
	FocuserTask	focusertask(ri, focusposition);;

	// if the filter name is specified, get the filterwheel from the
	// instrument and set the filter
	FilterwheelTask	filterwheeltask(ri, filtername);

	// if the temperature is set, and the ccd has a cooler, lets
	// start the cooler
	CoolerTask	coolertask(ri, temperature);;

	// now wait for all tasks to complete
	focusertask.wait();
	filterwheeltask.wait();
	try {
		coolertask.wait();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ignoring exeption: %s", x.what());
	}

	// prepare an exposure object
	ccdtask.frame(frame);
	ccdtask.binning(binning);
	ccdtask.exposuretime(exposuretime);
	ccdtask.purpose(purpose);

	// check whether the remote camera already has an exposed image,
	// in which case we want to cancel it
	ccdtask.available();

	// start a sequence of images
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting %d images", nImages);
	int	imagecounter = 0;
	while (imagecounter < nImages) {
		// start the exposure
		debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure %d",
			imagecounter);
		ccdtask.start();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure initiated, waiting");

		// wait for the exposure to complete
		try {
			ccdtask.wait();
		} catch (const BadState& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "badstate wait: %s",
				x.what());
			throw std::runtime_error(x.what());
		} catch (const DeviceException& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"device exception wait: %s", x.what());
			throw std::runtime_error(x.what());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image complete");

		// get the image data
		ImagePrx	image;
		try {
			image = ccd->getImage();
		} catch (const BadParameter& x) {
			std::string	msg = stringprintf(
				"bad parameter in getImage: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			std::cerr << msg << std::endl;
			throw std::runtime_error(msg);
		} catch (const BadState& x) {
			std::string	msg = stringprintf(
				"bad state in getImage: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			std::cerr << msg << std::endl;
			throw std::runtime_error(msg);
		} catch (const DeviceException& x) {
			std::string	msg = stringprintf(
				"device exception in getImage: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			std::cerr << msg << std::endl;
			throw std::runtime_error(msg);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got image: %dx%d",
			image->size().width, image->size().height);

		// convert image to an astro::image::imagePtr
		astro::image::ImagePtr	imageptr = convert(image);

		// add the instrument information (because that was not
		// available to the server)
		if (!imageptr->hasMetadata(std::string("INSTRUME"))) {
			imageptr->setMetadata(astro::io::FITSKeywords::meta(
				std::string("INSTRUME"), instrumentname));
		}

		// add the project information to the 
		if (0 == project.size()) {
			// try to get the default projec name from the 
			// config database
			if (config->has("global", "project", "default")) {
				project = config->get("global", "project", "default");
			}
		}
		if (project.size() > 0) {
			imageptr->setMetadata(astro::io::FITSKeywords::meta(
				std::string("PROJECT"), project));
		}

		// write the image to the repository
		try {
			ImageRepoPtr	repo = imagerepos->repo(reponame);
			repo->save(imageptr);
		} catch (std::exception& x) {
			std::string	msg = stringprintf("cannot save "
				"image: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		}

		// get rid of the image on the server side
		image->remove();

		// increase the counter
		imagecounter++;
	}

	// turn of the cooler to save energy, this is done by the destructor
	return EXIT_SUCCESS;
}

} // namespace snowimages
} // namespace app
} // namespace snowstar

int	main(int argc, char *argv[]) {
	int	rc = astro::main_function<snowstar::app::snowimages::main>(argc,
			argv);
	snowstar::CommunicatorSingleton::release();
	return rc;
}
