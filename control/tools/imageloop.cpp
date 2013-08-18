/*
 * imageloop.cpp -- program to retrieve images from a camera in a loop
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <stdexcept>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroIO.h>
#include <AstroFormat.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <AstroDevice.h>
#include <AstroLoop.h>

using namespace astro;
using namespace astro::io;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::image::filter;
using namespace astro::task;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -ad? ] [ -m module ] [ -C cameraid ] [ -c ccdid ] [ -n nimages ] [ -p period ] [ -E targetmean ] [ -e exposuretime ] [ -w width ] [ -h height ] [ -x xoffset ] [ -y yoffset ] [ -o directory ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d           increase deug level" << std::endl;
	std::cout << "  -m module    load camera module" << std::endl;
	std::cout << "  -C cameraid  which camera to use, default 0" << std::endl;
	std::cout << "  -c ccdid     which ccd to use, default 0" << std::endl;
	std::cout << "  -n nimages   number of images to retrieve" << std::endl;
	std::cout << "  -p period    image period" << std::endl;
	std::cout << "  -w width     width of image rectangle" << std::endl;
	std::cout << "  -h height    height of image rectangle" << std::endl;
	std::cout << "  -x xoffset   horizontal offset of image rectangle"
		<< std::endl;
	std::cout << "  -y yoffset   vertical offset of image rectangle"
		<< std::endl;
	std::cout << "  -n images    number of images, 0 means never stop"
		<< std::endl;
	std::cout << "  -o outdir    directory where files should be placed"
		<< std::endl;
	std::cout << "  -t           use timestamps as filenames" << std::endl;
	std::cout << "  -e time      (initial) exposure time, modified later if target mean set" << std::endl;
	std::cout << "  -E mean      attempt to vary the exposure time in such a way that" << std::endl;
	std::cout << "               that the mean pixel value stays close to mean" << std::endl;
	std::cout << "  -?           display this help message" << std::endl;
}

static const double	a = 0.5;

double	scalefactor(double x) {
	double	y = x - 1;
	double	result =  y * (1 - a * exp(-y * y)) + 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "x = %f, scalefactor = %f", x, result);
	return result;
}

int	main(int argc, char *argv[]) {
std::cout << "number of arguments: " << argc << std::endl;
	debugtimeprecision = 3;
	debugthreads = 1;
	int	c;
	unsigned int	width = 0;
	unsigned int	height = 0;
	unsigned int	xoffset = 0;
	unsigned int	yoffset = 0;
	const char	*outpath = ".";
	unsigned int	cameraid = 0;
	unsigned int	ccdid = 0;
	unsigned int	nImages = 1;
	double	exposuretime = 0.1;
	const char	*modulename = "uvc";
	double		targetmean = 0;
	unsigned int	period = 1; // one second
	bool	align = false;	// indicates whether exposures should be
				// synchronized with the clock
	bool	timestamped = false;
	while (EOF != (c = getopt(argc, argv,
			"adw:x:y:w:h:o:C:c:n:e:E:m:p:t?"))) {
		switch (c) {
		case 'a':
			align = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'h':
			height = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'x':
			xoffset = atoi(optarg);
			break;
		case 'y':
			yoffset = atoi(optarg);
			break;
		case 'o':
			outpath = optarg;
			break;
		case 'C':
			cameraid = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'n':
			nImages = atoi(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'm':
			modulename = optarg;
			break;
		case 'E':
			targetmean = atof(optarg);
			break;
		case 'p':
			period = atoi(optarg);
			break;
		case 't':
			timestamped = true;
			break;
		}
	}

	// if E is set, and the initial exposure time is zero, then
	// we should change it to something more reasonable
	if ((0 != targetmean) && (exposuretime == 0)) {
		std::string	msg("cannot change exposure time dynamically "
			"starting from 0");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// make sure the target directory exists
	FITSdirectory::filenameformat	format
		= (timestamped)	? FITSdirectory::TIMESTAMP
				: FITSdirectory::COUNTER;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "format: %d", format);
	FITSdirectory	directory(outpath, format);
	if (timestamped) {
		if (period >= 60) {
			directory.timestampformat("%H%M");
		} else {
			directory.timestampformat("%H%M%S");
		}
	}

	// load the module
	Repository      repository;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "recovering module '%s'", modulename);
	ModulePtr       module = repository.getModule(modulename);
	module->open();

        // get the camera
	DeviceLocatorPtr        locator = module->getDeviceLocator();
	std::vector<std::string>        cameras = locator->getDevicelist();
	if (cameraid >= cameras.size()) {
		std::string	msg = stringprintf("camera id %d out of range",
			cameraid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}

	// get the camera
	std::string	cameraname = cameras[cameraid];
	CameraPtr	camera = locator->getCamera(cameraname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got camera %s", cameraname.c_str());

	// get the ccd
        CcdPtr  ccd = camera->getCcd(ccdid);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd: %s",
                ccd->getInfo().toString().c_str());

	// find a fitting image rectangle
        if (width == 0) {
                width = ccd->getInfo().size().width();
        }
        if (height == 0) {
                height = ccd->getInfo().size().height();
        }
        ImageRectangle  imagerectangle = ccd->getInfo().clipRectangle(
                ImageRectangle(ImagePoint(xoffset, yoffset),
                        ImageSize(width, height)));
	Exposure	exposure(imagerectangle, exposuretime);

	// now create the Loop object
	Loop	loop(ccd, exposure, directory);
	loop.period(period);
	loop.nImages(nImages);
	loop.align(align);
	if (targetmean > 0) {
		loop.targetmean(targetmean);
	}

	// run the loop
	loop.execute();

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s terminated: %s",
			argv[0], x.what());
	}
}
