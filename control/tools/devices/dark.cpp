/*
 * dark.cpp -- make a dark
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
#include <AstroCamera.h>
#include <AstroLoader.h>
#include <AstroDevice.h>

using namespace astro;
using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;
using namespace astro::camera;
using namespace astro::module;
using namespace astro::device;

namespace astro {
namespace app {
namespace dark {

/**
 * \brief display a help message for the dark program
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << "Get a number of images from a CCD and consolidate them "
		"into a dark image" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -C,--camera=<cameraid>    "
		"use camera with id <cameraid>" << std::endl;
	std::cout << "    -c,--ccd=<ccdid>          "
		"use ccd with id <ccdid>" << std::endl;
	std::cout << "    -d,--debug                "
		"increase debug level" << std::endl;
	std::cout << "    -e,--exposure=<time>      "
		"set exposure time to <time>" << std::endl;
	std::cout << "    -m,--module=<module>      use module <module>"
		<< std::endl;
	std::cout << "    -n,--number=<nimages>     "
		"build dark composed of <nimages> individual images"
		<< std::endl;
	std::cout << "    -o,--outfile=<outfile>    "
		"filename of the output dark image" << std::endl;
	std::cout << "    -t,--temperature=<temp>   "
		"cool CCD to temperature <temp>" << std::endl;
	std::cout << "    -h,-?,--help              "
		"show this help message" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "camera",		required_argument,	NULL,	'C' }, /* 0 */
{ "ccd",		required_argument,	NULL,	'c' }, /* 1 */
{ "debug",		no_argument,		NULL,	'd' }, /* 2 */
{ "exposure",		required_argument,	NULL,	'e' }, /* 3 */
{ "module",		required_argument,	NULL,	'm' }, /* 4 */
{ "number",		required_argument,	NULL,	'n' }, /* 5 */
{ "outfile",		required_argument,	NULL,	'o' }, /* 6 */
{ "temperature",	required_argument,	NULL,	't' }, /* 7 */
{ "help",		no_argument,		NULL,	'h' }, /* 8 */
{ NULL,			0,			NULL,	 0  }, /* 9 */
};

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a number of images from a CCD and produces a dark image
 * from them.
 */
int	main(int argc, char *argv[]) {
	Exposure	exposure;
	double	exposuretime = 1;
	unsigned int	nimages = 3;
	float	temperature = 0;
	const char	*outfilename = NULL;
	int	c;
	int	longindex;
	int	cameranumber = 0;
	int	ccdid = 0;
	const char	*modulename = "uvc";
	while (EOF != (c = getopt_long(argc, argv, "do:t:n:h?m:C:c:e:",
		longopts, &longindex)))
		switch (c) {
		case 'C':
			cameranumber = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'm':
			modulename = optarg;
			break;
		case 'n':
			nimages = atoi(optarg);
			break;
		case 'o':
			outfilename = optarg;
			break;
		case 't':
			temperature = atof(optarg) + 273.1;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// get module
	ModuleRepositoryPtr	repository = getModuleRepository();
	ModulePtr	module = repository->getModule(modulename);
	module->open();

	// get camera and ccd
	DeviceLocatorPtr	locator = module->getDeviceLocator();
	std::vector<std::string>	cameras = locator->getDevicelist();
	CameraPtr	camera = locator->getCamera(cameras[cameranumber]);
	CcdPtr	ccd = camera->getCcd(ccdid);
	exposure.frame(ccd->getInfo().getFrame());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "frame: %s",
		exposure.frame().toString().c_str());

	// launch the darkframeprocess
	DarkFrameProcess	dfp(ccd);
	dfp.setTemperature(temperature);
	dfp.setNimages(nimages);
	dfp.setExposuretime(exposuretime);
	ImagePtr	dark = dfp.get();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image %d x %d generated",
		dark->size().width(), dark->size().height());

	if (outfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "outfile: %s", outfilename);
		unlink(outfilename);
		FITSout	outfile(outfilename);
		outfile.write(dark);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image written to %s",
			outfilename);
	}

	return EXIT_SUCCESS;
}

} // namespace dark
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::dark::main>(argc, argv);
}
