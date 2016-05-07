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

namespace astro {
namespace app {
namespace getimages {

static void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ]" << std::endl;
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
	std::cout << " -i,--instrument=<INS> use instrument named INS";
	std::cout << std::endl;
	std::cout << " -n,--number=<n>       take <n> exposures with these "
			"settings";
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
{ "instrument",		required_argument,	NULL,	'i' }, /*  7 */
{ "number",		required_argument,	NULL,	'n' }, /*  8 */
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
	unsigned short	focusposition = 0;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:c:de:f:F:hi:n:p:r:t:",
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
		case 'i':
			instrumentname = optarg;
			break;
		case 'n':
			nImages = atoi(optarg);
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
			case 10:
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"rectangle %s", optarg);
				frame = ImageRectangle(optarg);
				break;
			default:
				debug(LOG_DEBUG, DEBUG_LOG, 0, "ignore %d",
					longindex);
				break;
			}
		}
	}

	// get the configuration
	ConfigurationPtr	config = Configuration::get();

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
		throw std::runtime_error("instrument name not set");
	}
	InstrumentConfigurationPtr	instruments
		= InstrumentConfiguration::get(config);
	InstrumentPtr	instrument = instruments->instrument(instrumentname);

	// make sure we have a repository, because we would not know
	// where to store the images otherwise
	if (0 == reponame.size()) {
		throw std::runtime_error("repository name not set");
	}
	ImageRepoConfigurationPtr	imagerepos
		= ImageRepoConfiguration::get(config);
	ImageRepoPtr	repo = imagerepos->repo(reponame);

	// get the devices
	CameraPtr	camera = instrument->camera();
	CcdPtr		ccd = instrument->ccd();

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
	if ((focusposition > 0) && (instrument->has(DeviceName::Focuser))) {
		FocuserPtr	focuser = instrument->focuser();
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
		&& (instrument->has(DeviceName::Filterwheel))) {
		FilterWheelPtr	filterwheel = instrument->filterwheel();
		filterwheel->select(filtername);
		filterwheel->wait(20);
	}

	// if the temperature is set, and the ccd has a cooler, lets
	// start the cooler
	CoolerPtr	cooler(NULL);
	if ((temperature == temperature)
		&& (instrument->has(DeviceName::Cooler))) {
		double	absolute = 273.15 + temperature;
		if (absolute < 0) {
			throw std::runtime_error("bad temperature");
		}
		cooler = instrument->cooler();
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
				"set: %.1f, actual: %1.f, delta: %.1f",
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
	if (Exposure::exposed == ccd->exposureStatus()) {
		ccd->cancelExposure();
		while (Exposure::idle != ccd->exposureStatus()) {
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
		}
		(*imageptr)->setMetadata(
			FITSKeywords::meta(std::string("INSTRUME"),
				instrument->name()));
		repo->save(*imageptr);
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
