/*
 * getstream.cpp -- get a stream of images from a camera
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroCamera.h>
#include <AstroConfig.h>
#include <AstroIO.h>
#include <AstroDiscovery.h>
#include <AstroLoader.h>

using namespace astro::camera;
using namespace astro::config;
using namespace astro::project;
using namespace astro::io;
using namespace astro::discover;
using namespace astro::module;

namespace astro {
namespace app {
namespace getstream {

static void	usage(const char *progname) {
	Path	path(progname);
	std::string	p = path.basename();
	std::cout << "uage: " << p << " [ options ] instrument" << std::endl;
	std::cout << "retrieves a stream of images from an instrument" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--binning=XxY         use XxY binning mode (default 1x1)" << std::endl;
	std::cout << " -c,--config=<cfg>        use configuration database in file <cfg>" << std::endl;
	std::cout << " -d,--debug               increase debug level" << std::endl;
	std::cout << " -e,--exposure=<e>        use exposure time <e> (seconds)" << std::endl;
	std::cout << " -f,--filter=<f>          use filter  numbered <f>" << std::endl;
	std::cout << " -h,--help                display this help message and exit" << std::endl;
	std::cout << " -n,--number=<n>          retrieve exactly <n> images" << std::endl;
	std::cout << " -p,--purpose=<p>         images have purpose <p>, i.e. one of light, dark" << std::endl;
	std::cout << "                          or flat, default is light" << std::endl;
	std::cout << " --rectangle=<rect>       expose only the subrectangle specified by <rect>" << std::endl;
	std::cout << "                          <rect> must have format widthxheight@(xoffset,yoffset)" << std::endl;
	std::cout << " -r,--repo=<rep>          write images to repository <repop>" << std::endl;
	std::cout << " -t,--temperature=<temp>  cool CCD to temperature <temp>" << std::endl;
}

static struct option	longopts[] = {
{ "binning",		required_argument,	NULL,	'b' },
{ "config",		required_argument,	NULL,	'c' },
{ "debug",		no_argument,		NULL,	'd' },
{ "exposure",		required_argument,	NULL,	'e' },
{ "filter",		required_argument,	NULL,	'f' },
{ "help",		no_argument,		NULL,	'h' },
{ "number",		required_argument,	NULL,	'n' },
{ "purpose",		required_argument,	NULL,	'p' },
{ "repo",		required_argument,	NULL,	'r' },
{ "rectangle",		required_argument,	NULL,	 1  },
{ "temperature",	required_argument,	NULL,	't' },
{ NULL,			0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	debugthreads = 1;
	Exposure	exposure;
	unsigned int	nImages = 1;
	std::string	reponame;
	std::string	filtername;
	int	c;
	int	longindex;
	double	temperature = std::numeric_limits<double>::quiet_NaN();
	while (EOF != (c = getopt_long(argc, argv, "b:c:de:f:hn:p:r:t:?",
		longopts, &longindex)))
		switch (c) {
		case 'b':
			exposure.mode(Binning(optarg));
			break;
		case 'c':
			Configuration::set_default(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposure.exposuretime(atof(optarg));
			break;
		case 'f':
			filtername = std::string(optarg);
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'n':
			nImages = atoi(optarg);
			break;
		case 'p':
			exposure.purpose(Exposure::string2purpose(optarg));
			break;
		case 'r':
			reponame = std::string(optarg);
			break;
		case 't':
			temperature = atof(optarg);
			break;
		case 1:
			exposure.frame(ImageRectangle(optarg));
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// next argument must be instrument name or help
	if (optind >= argc) {
		std::cerr << "missing instrument name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	instrumentname(argv[optind++]);

	// get the configuration
	ConfigurationPtr	config = Configuration::get();

	// backend for instruments
	InstrumentBackend	instrumentbackend(config->database());
        InstrumentPtr   instrument = instrumentbackend.get(instrumentname);

	// get the image repository
	ImageRepoPtr	repo(NULL);
	if (reponame.size() > 0) {
		ImageRepoConfigurationPtr	imagerepos
			= ImageRepoConfiguration::get(config);
		repo = imagerepos->repo(reponame);
	}

	// prepare a repository from which we can extract the devices
	ModuleRepositoryPtr	repository = getModuleRepository();
	Devices		devices(repository);

	// get the devices
	CameraPtr	camera = devices.getCamera(instrument->getCamera(0).deviceurl());
	CcdPtr		ccd = devices.getCcd(instrument->getCcd(0).deviceurl());

	// If temperature is set, and a cooler is present, initialize the
	// cooler and wait until temperature is reached
	CoolerPtr	cooler(NULL);
	if ((temperature == temperature)
		&& (instrument->hasCooler())) {
		double	absolute = 273.15 + temperature;
		if (absolute < 0) {
			std::string	msg
				= stringprintf("illegal temperature: %f",
					temperature);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		cooler = devices.getCooler(instrument->getCooler(0).deviceurl());
		cooler->setTemperature(absolute);
		cooler->setOn(true);

		double	delta;
		do {
			double	actual = cooler->getActualTemperature();
			delta = fabs(absolute - actual);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"set: %.1f, actual: %.1f, delta: %.1f",
				absolute, actual, delta);
		} while (delta > 1);
	}

	// if the instrument has a filter wheel, we get a pointer to it
	// and try 
	FilterWheelPtr	filterwheel(NULL);
	if (instrument->hasFilterWheel()) {
		filterwheel = devices.getFilterWheel(
			instrument->getFilterWheel(0).deviceurl());
		filterwheel->wait(20);
		if (filtername.size() > 0) {
			filterwheel->select(filtername);
			filterwheel->wait(20);
		}
	}

	// start the stream
	unsigned int	imagesRetrieved = 0;
	ccd->startStream(exposure);
	while (imagesRetrieved < nImages) {
		ImagePtr	image = ccd->getEntry(true).image;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got image[%u] %s",
			++imagesRetrieved,
			image->size().toString().c_str());
		if (!image->hasMetadata(std::string("INSTRUME"))) {
			image->setMetadata(FITSKeywords::meta(
				std::string("INSTRUME"), instrument->name()));
		}
		// do something about the image
		if (repo) {
			repo->save(image);
		}
	}
	// stop the stream
	ccd->stopStream();

	// find out how many images were dropped
	if (ccd->dropped() > 0) {
		std::cerr << "images dropped: " << ccd->dropped() << std::endl;
	}

	// turn off the cooler
	if (cooler) {
		cooler->setOn(false);
	}
	
	return EXIT_SUCCESS;
}

} // namespace getstream
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::getstream::main>(argc, argv);
}
