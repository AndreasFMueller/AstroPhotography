/*
 * getimages.cpp -- tool to retrieve a sequence of images from a camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <AstroConfig.h>
#include <AstroDiscovery.h>
#include <AstroProject.h>
#include <AstroIO.h>
#if ENABLE_CORBA
#include <OrbSingleton.h>
#endif /* ENABLE_CORBA */
#include <AstroUtils.h>

using namespace astro;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::image;
using namespace astro::io;
using namespace astro::config;
using namespace astro::project;
using namespace astro::discover;

namespace astro {
namespace app {
namespace getimages {

static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage: " << path.basename() << " [ options ] <INSTRUMENT>"
		<< std::endl;
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
	std::cout << " -o,--out=<prefix>     write the image to a file created from the prefix";
	std::cout << std::endl;
	std::cout << "                       by appending a number and the suffix .fits";
	std::cout << std::endl;
	std::cout << "                       No output is written if the prefix is not given,";
	std::cout << std::endl;
	std::cout << " -p,--purpose=<p>      images have purpose <p>, i.e. one "
		"of light, dark";
	std::cout << std::endl;
	std::cout << "                       or flat";
	std::cout << std::endl;
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

static struct option	longopts[] = {
/* name			argument?		int*	int */
{ "binning",		required_argument,	NULL,	'b' }, /*  0 */
{ "config",		required_argument,	NULL,	'c' }, /*  1 */
{ "debug",		no_argument,		NULL,	'd' }, /*  2 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  3 */
{ "filter",		required_argument,	NULL,	'f' }, /*  4 */
{ "focus",		required_argument,	NULL,	'F' }, /*  5 */
{ "help",		no_argument,		NULL,	'h' }, /*  6 */
{ "number",		required_argument,	NULL,	'n' }, /*  7 */
{ "out",		required_argument,	NULL,	'o' }, /*  8 */
{ "purpose",		required_argument,	NULL,	'p' }, /*  9 */
{ "rectangle",		required_argument,	NULL,	 1  }, /* 10 */
{ "repo",		required_argument,	NULL,	'r' }, /* 11 */
{ "temperature",	required_argument,	NULL,	't' }, /* 12 */
{ NULL,			0,			NULL,    0  }
};

/**
 * \brief Main method for the getimages program
 */
int	main(int argc, char *argv[]) {
	unsigned int	nImages = 1;
	std::string	instrumentname;
	float	exposuretime = 1.; // default exposure time: 1 second
	double	temperature = std::numeric_limits<double>::quiet_NaN(); 

	// initialize the orb in case we want to use the net module
#if ENABLE_CORBA
	Astro::OrbSingleton	orb(argc, argv);
#endif /* ENABLE_CORBA */
	debugtimeprecision = 3;
	debugthreads = 1;
	Binning	binning;
	std::string	filtername;
	std::string	reponame;
	ImageRectangle	frame;
	Exposure::purpose_t	purpose = Exposure::light;
	long	focusposition = 0;
	std::string	prefix;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:de:f:F:hn:o:p:r:t:",
		longopts, &longindex))) {
		switch (c) {
		case 'b':
			binning = Binning(optarg);
			break;
		case 'c':
			Configuration::set_default(optarg);
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
		case 'o':
			prefix = std::string(optarg);
			break;
		case 'p':
			purpose = Exposure::string2purpose(optarg);
			break;
		case 'r':
			reponame = optarg;
			break;
		case 't':
			temperature = std::stod(optarg);
			break;
		case 1:
			switch (longindex) {
			case 9:
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"rectangle %s", optarg);
				frame = ImageRectangle(optarg);
				break;
			default:
				debug(LOG_DEBUG, DEBUG_LOG, 0, "ignore %d",
					longindex);
				break;
			}
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// next argument must be the instrument name
	if (optind >= argc) {
		throw std::runtime_error("missing instrument name");
	}
	instrumentname = std::string(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument name: %s",
		instrumentname.c_str());

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
		throw std::runtime_error("instrument name not set");
	}

	// get the configuration
	ConfigurationPtr	config = Configuration::get();
	InstrumentBackend	instrumentbackend(config->database());
	InstrumentPtr	instrument = instrumentbackend.get(instrumentname);

	// make sure we have a repository, because we would not know
	// where to store the images otherwise
	ImageRepoPtr	repo;
	if (0 == reponame.size()) {
		std::cerr << "Warning: no repository set, make sure output prefix is set";
		std::cerr << std::endl;;
	} else {
		ImageRepoConfigurationPtr	imagerepos
			= ImageRepoConfiguration::get(config);
		repo = imagerepos->repo(reponame);
	}

	// get the components
	std::string	ccdurl = instrument->getCcd(0).deviceurl();
	std::string	cameraurl = instrument->getCamera(0).deviceurl();

	// get the devices
	Repository	repository;
	Devices		devices(repository);
	CameraPtr	camera = devices.getCamera(cameraurl);
	CcdPtr	ccd = devices.getCcd(ccdurl);

	// get the image repository
	if ((frame.size().width() == 0) || (frame.size().height() == 0)) {
		frame = ccd->getInfo().getFrame();
	} else {
		frame = ccd->getInfo().clipRectangle(frame);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image rectangle: %s",
		frame.toString().c_str());

	// if the focuser is specified, we try to get it and then set
	// the focus value
	if ((focusposition > 0) && (instrument->hasFocuser())) {
		std::string	focuserurl
			= instrument->getFocuser(0).deviceurl();
		FocuserPtr	focuser = devices.getFocuser(focuserurl);
		focuser->set(focusposition);
		while (focuser->current() != focusposition) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"current = %hu, focus = %hu",
				focuser->current(), focusposition);
			usleep(100000);
		}
	}

	// if the filter name is specified, get the filterwheel from the
	// instrument and set the filter
	if ((filtername.size() > 0)
		&& (instrument->hasFilterWheel())) {
		std::string	filterwheelurl
			= instrument->getFilterWheel(0).deviceurl();
		FilterWheelPtr	filterwheel
			= devices.getFilterWheel(filterwheelurl);
		filterwheel->select(filtername);
		filterwheel->wait(20);
	}

	// if the temperature is set, and the ccd has a cooler, lets
	// start the cooler
	CoolerPtr	cooler(NULL);
	if ((temperature == temperature)
		&& (instrument->hasCooler())) {
		double	absolute = 273.15 + temperature;
		if (absolute < 0) {
			std::string	msg = stringprintf("illegal temperature: %f", temperature);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		cooler = devices.getCooler(instrument->getCooler().deviceurl());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing the cooler");
		cooler->setTemperature(absolute);
		cooler->setOn(true);
		// wait until the temperature is within 1 degree of the
		// set temperature
		double	delta;
		do {
			sleep(1);
			double	actual = cooler->getActualTemperature();
			delta = fabs(absolute - actual);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"set: %.1f, actual: %.1f, delta: %.1f",
				absolute, actual, delta);
		} while (delta > 1);
	}

	// prepare an exposure object
	Exposure	exposure(frame, exposuretime);
	exposure.purpose(purpose);
	exposure.shutter((purpose == Exposure::dark)
				? Shutter::CLOSED : Shutter::OPEN);
	exposure.mode(binning);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s",
		exposure.toString().c_str());

	// check whether the remote camera already has an exposed image,
	// in which case we want to cancel it
	if (CcdState::exposed == ccd->exposureStatus()) {
		ccd->cancelExposure();
		while (CcdState::idle != ccd->exposureStatus()) {
			usleep(100000);
		}
	}

	// start the exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");
	ccd->startExposure(exposure);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure initiated, waiting");

	// read all images
	ImageSequence	images = ccd->getImageSequence(nImages);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieved %d images", images.size());

	// turn of the cooler to save energy
	if (cooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "turning cooler off");
		cooler->setOn(false);
	}

	// write the images to the repository
	ImageSequence::const_iterator	imageptr;
	int	counter = 0;
	for (imageptr = images.begin(); imageptr != images.end(); imageptr++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding image");
		if ((*imageptr)->hasMetadata(std::string("INSTRUME"))) {
			(*imageptr)->setMetadata(
				FITSKeywords::meta(std::string("INSTRUME"),
					instrument->name()));
		}
		if (repo) {
			repo->save(*imageptr);
		} else {
			if (0 == prefix.size()) {
				std::cerr << "no prefix, images lost"
					<< std::endl;
				return EXIT_FAILURE;
			}
			std::string	filename = stringprintf("%s%03d.fits",
				prefix.c_str(), counter++);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "writing image %s",
				filename.c_str());
			FITSout	out(filename);
			out.write(*imageptr);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d images written", counter);

	return EXIT_SUCCESS;
}

} // namespace getimages
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return main_function<astro::app::getimages::main>(argc, argv);
}
