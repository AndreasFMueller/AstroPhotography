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
#include <stacktrace.h>

using namespace astro::calibration;
using namespace astro::io;
using namespace astro::image;
using namespace astro::camera;
using namespace astro::module;
using namespace astro::device;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ]" << std::endl;
	std::cout << "Computes a consolidated flat image from a set of images"
		<< std::endl;
	std::cout << "recorded by this program" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d             increase debug level" << std::endl;
	std::cout << "  -e <time>      set exposure time to <time>" << std::endl;
	std::cout << "  -n <nimages>   build dark composed of <nimages> individual images" << std::endl;
	std::cout << "  -t <temp>      cool CCD to temperature <temp>" << std::endl;
	std::cout << "  -D <dark>      use the dark frame from file <dark> to correct all" << std::endl;
	std::cout << "                 prior to inclusion into the flat frame computation" << std::endl;
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
int	flat_main(int argc, char *argv[]) {
	Exposure	exposure;
	double	exposuretime = 1;
	unsigned int	nimages = 3;
	float	temperature = 0;
	const char	*outfilename = NULL;
	int	c;
	int	cameranumber = 0;
	int	ccdid = 0;
	const char	*modulename = "uvc";
	const char	*darkfilename = NULL;
	while (EOF != (c = getopt(argc, argv, "do:t:n:h?m:C:c:e:D:")))
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

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::flat_main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "terminated by " << typeid(x).name() << ": ";
		std::cerr << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
