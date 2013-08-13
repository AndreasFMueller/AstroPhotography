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

using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;
using namespace astro::camera;
using namespace astro::module;
using namespace astro::device;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ] darkimages"
		<< std::endl;
	std::cout <<"Computes a consolidated dark image from a set of images"
		<< std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d             increase debug level" << std::endl;
	std::cout << "  -e <time>      set exposure time to <time>" << std::endl;
	std::cout << "  -n <nimages>   build dark composed of <nimages> individual images" << std::endl;
	std::cout << "  -t <temp>      cool CCD to temperature <temp>" << std::endl;
	std::cout << "  -h, -?         show this help message" << std::endl;
	std::cout << "  -o outfile     filename of the output dark image"
		<< std::endl;
}

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
	int	cameranumber = 0;
	int	ccdid = 0;
	const char	*modulename = "uvc";
	while (EOF != (c = getopt(argc, argv, "do:t:n:h?m:C:c:e:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
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
			temperature = atof(optarg);
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
		}

	// get module
	Repository	repository;
	ModulePtr	module = repository.getModule(modulename);
	module->open();

	// get camera and ccd
	DeviceLocatorPtr	locator = module->getDeviceLocator();
	std::vector<std::string>	cameras = locator->getDevicelist();
	CameraPtr	camera = locator->getCamera(cameras[cameranumber]);
	CcdPtr	ccd = camera->getCcd(ccdid);
	exposure.frame = ccd->getInfo().getFrame();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "frame: %s", exposure.frame.toString().c_str());

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

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "makedark tool terminated by "
			"exception: %s", x.what());
		std::cerr << "makedark tool terminated: " << x.what()
			<< std::endl;
	}
}
