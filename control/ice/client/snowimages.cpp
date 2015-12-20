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

static void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] <service> <INSTRUMENT>"
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
{ "ccd",		required_argument,	NULL,	'C' }, /*  . */
{ "config",		required_argument,	NULL,	'c' }, /*  1 */
{ "debug",		no_argument,		NULL,	'd' }, /*  2 */
{ "exposure",		required_argument,	NULL,	'e' }, /*  3 */
{ "filter",		required_argument,	NULL,	'f' }, /*  4 */
{ "focus",		required_argument,	NULL,	'F' }, /*  5 */
{ "help",		no_argument,		NULL,	'h' }, /*  6 */
{ "number",		required_argument,	NULL,	'n' }, /*  8 */
{ "purpose",		required_argument,	NULL,	'p' }, /*  9 */
{ "rectangle",		required_argument,	NULL,	 1  }, /* 10 */
{ "repo",		required_argument,	NULL,	'r' }, /* 11 */
{ "temperature",	required_argument,	NULL,	't' }, /* 12 */
{ NULL,			0,			NULL,    0  }
};

int	main(int argc, char *argv[]) {
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

	// focus position
	unsigned short	focusposition = 0;

	// ccd index
	int	ccd_index = 0;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "b:C:c:de:f:F:hn:p:r:t:",
		longopts, &longindex))) {
		switch (c) {
		case 'b':
			binning = optarg;
			break;
		case 'C':
			ccd_index = std::stoi(optarg);
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
		case 'p':
			purpose = astro::camera::Exposure::string2purpose(optarg);
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
					"rectangle: %s", optarg);
				frame = astro::image::ImageRectangle(optarg);
				break;
			default:
				debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring %d",
					longindex);
				break;
			}
		}
	}

	// server and instrument name
	if (optind >= argc) {
		throw std::runtime_error("service name missing");
	}
	astro::ServerName	servername(argv[optind++]);
	if (optind >= argc) {
		throw std::runtime_error("instrument name missing");
	}
	std::string	instrumentname(argv[optind++]);

	// get the configuration
	ConfigurationPtr	config = Configuration::get();

	// check whether we have an instrument
	if (0 == instrumentname.size()) {
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
		if (config->hasglobal("repository", "default")) {
			reponame = config->global("repository", "default");
		} else {
			throw std::runtime_error("repository name not set");
		}
	}
	astro::config::ImageRepoConfigurationPtr	imagerepos
		= astro::config::ImageRepoConfiguration::get(config);
	ImageRepoPtr	repo = imagerepos->repo(reponame);

	// Create a remote instrument
	RemoteInstrument	ri(instruments, instrumentname);
debug(LOG_DEBUG, DEBUG_LOG, 0, "got remote instrument");

	// get the Ccd
	snowstar::CcdPrx	ccd = ri.ccd(ccd_index);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd");

	// get the image repository
	CcdTask	ccdtask(ccd);
	ccdtask.frame(frame);

	// if the focuser is specified, we try to get it and then set
	// the focus value
	if ((focusposition > 0) && (ri.has(InstrumentFocuser))) {
		snowstar::FocuserPrx	focuser = ri.focuser();
		FocuserTask	focusertask(focuser, focusposition);
		focusertask.wait();
	}

	// if the filter name is specified, get the filterwheel from the
	// instrument and set the filter
	if ((filtername.size() > 0)
		&& (ri.has(InstrumentFilterWheel))) {
		snowstar::FilterWheelPrx	filterwheel = ri.filterwheel();
		FilterwheelTask	filterwheeltask(filterwheel, filtername);
		filterwheeltask.wait();
	}

	// if the temperature is set, and the ccd has a cooler, lets
	// start the cooler
	snowstar::CoolerPrx	cooler;
	if (ri.has(InstrumentCooler)) {
		cooler = ri.cooler();
	}
	if (temperature == temperature) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting temperature to %.1f",
			temperature);
		CoolerTask	coolertask(cooler, temperature);
		coolertask.wait();
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
		ccdtask.wait();

		// get the image data
		ImagePrx	image = ccd->getImage();

		// convert image to an astro::image::imagePtr
		astro::image::ImagePtr	imageptr = convert(image);

		// add the instrument information (because that was not
		// available to the server)
		if (!imageptr->hasMetadata(std::string("INSTRUME"))) {
			imageptr->setMetadata(astro::io::FITSKeywords::meta(
				std::string("INSTRUME"), instrumentname));
		}

		// write the image to the repository
		repo->save(imageptr);

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
	return astro::main_function<snowstar::app::snowimages::main>(argc, argv);
}
