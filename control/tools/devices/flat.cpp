/*
 * flat.cpp -- make a flat
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

using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;
using namespace astro::camera;
using namespace astro::module;
using namespace astro::device;

namespace astro {
namespace app {
namespace flat {

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ]" << std::endl;
	std::cout << std::endl;
	std::cout << "Computes a consolidated flat image from a set of images"
		<< std::endl;
	std::cout << "recorded by this program" << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug               increase debug level"
		<< std::endl;
	std::cout << "    -e,--exposure=<time>     set exposure time to <time>"
		<< std::endl;
	std::cout << "    -n,--number=<nimages>    build dark composed of "
		"<nimages> individual images" << std::endl;
	std::cout << "    -t,--temperature <temp>  cool CCD to temperature "
		"<temp>" << std::endl;
	std::cout << "    -D,--dark=<dark>         use the dark frame from file"
		" <dark> to correct all" << std::endl;
	std::cout << "                             prior to inclusion into the "
		"flat frame computation" << std::endl;
	std::cout << "    -h,-?,--help             show this help message"
		<< std::endl;
	std::cout << "    -o,--outfile=<outfile>   filename of the output dark "
		"image" << std::endl;
}

static struct option	longopts[] = {
{ "camera",		required_argument,	NULL,	'C' }, /* 0 */
{ "ccd",		required_argument,	NULL,	'c' }, /* 1 */
{ "dark",		required_argument,	NULL,	'D' }, /* 2 */
{ "debug",		no_argument,		NULL,	'd' }, /* 3 */
{ "exposure",		required_argument,	NULL,	'e' }, /* 4 */
{ "module",		required_argument,	NULL,	'm' }, /* 5 */
{ "number",		required_argument,	NULL,	'n' }, /* 6 */
{ "outfile",		required_argument,	NULL,	'd' }, /* 7 */
{ "temperature",	required_argument,	NULL,	't' }, /* 8 */
{ "help",		no_argument,		NULL,	'h' }, /* 9 */
};

/**
 * \brief Main function for makedark tool 
 *
 * This tool takes a list of image names on the command line, reads them,
 * and produces a dark image from them.
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
	const char	*darkfilename = NULL;
	while (EOF != (c = getopt_long(argc, argv, "do:t:n:h?m:C:c:e:D:",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'D':
			darkfilename = optarg;
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'o':
			outfilename = optarg;
			break;
		case 'n':
			nimages = atoi(optarg);
			break;
		case 't':
			temperature = atof(optarg) + 273.1;
			break;
		case 'm':
			modulename = optarg;
			break;
		case 'C':
			cameranumber = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
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
	auto	repository = ModuleRepository::get();
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

	// get the dark frame
	ImagePtr	dark;
	if (darkfilename) {
		FITSin	in(darkfilename);
		dark = in.read();
	}

	// launch the darkframeprocess
	FlatFrameProcess	ffp(ccd, dark);
	ffp.setTemperature(temperature);
	ffp.setNimages(nimages);
	ffp.setExposuretime(exposuretime);
	ImagePtr	flat = ffp.get();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "flat image %d x %d generated",
		flat->size().width(), flat->size().height());

	if (outfilename) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "outfile: %s", outfilename);
		unlink(outfilename);
		FITSout	outfile(outfilename);
		outfile.write(flat);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark image written to %s",
			outfilename);
	}

	return EXIT_SUCCESS;
}

} // namespace flat
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::flat::main>(argc, argv);
}
