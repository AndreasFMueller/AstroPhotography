/*
 * imageloop.cpp -- program to retrieve images from a camera in a loop
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <stdexcept>
#include <debug.h>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroIO.h>
#include <Format.h>
#include <AstroFilter.h>

using namespace astro;
using namespace astro::io;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::image::filter;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -d? ] [ -m module ] [ -w width ] [ -h height ] [ -x xoffset ] [ -y yoffset ] [ -o directory ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d           increase deug level" << std::endl;
	std::cout << "  -m module    load camera module" << std::endl;
	std::cout << "  -w width     width of image rectangle" << std::endl;
	std::cout << "  -h height    height of image rectangle" << std::endl;
	std::cout << "  -x xoffset   horizontal offset of image rectangle"
		<< std::endl;
	std::cout << "  -y yoffset   vertical offset of image rectangle"
		<< std::endl;
	std::cout << "  -n images    number of images, 0 means never stop" << std::endl;
	std::cout << "  -o outdir    directory where files should be placed"
		<< std::endl;
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
	unsigned int	period = 1;
	while (EOF != (c = getopt(argc, argv, "dw:x:y:w:h:o:C:c:n:e:E:m:p:?")))
		switch (c) {
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
	FITSdirectory	directory(outpath);

	// load the module
	Repository      repository;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "recovering module '%s'", modulename);
	ModulePtr       module = repository.getModule(modulename);
	module->open();

        // get the camera
	CameraLocatorPtr        locator = module->getCameraLocator();
	std::vector<std::string>        cameras = locator->getCameralist();
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
                width = ccd->getInfo().size.width;
        }
        if (height == 0) {
                height = ccd->getInfo().size.height;
        }
        ImageRectangle  imagerectangle = ccd->getInfo().clipRectangle(
                ImageRectangle(ImagePoint(xoffset, yoffset),
                        ImageSize(width, height)));

	// find the first image time
	time_t	start = time(NULL);
	time_t	next = start;

	// now initialize exposure computation loop
	unsigned int	counter = 0;
	while ((counter++ < nImages) || (nImages == 0)) {
		// make sure the exposure time is not too long
		time_t	now = time(NULL);
		while (next <= now) {
			next += period;
		}
		if (exposuretime > (next - now)) {
			exposuretime = next - now;
		}

		// get an exposure
		debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure %s, time %fs",
			imagerectangle.toString().c_str(), exposuretime);
		// get an image with the current parameters
		Exposure	exposure(imagerectangle, exposuretime);
		ccd->startExposure(exposure);
		ImagePtr	image = ccd->getImage();
		directory.add(image);

		// compute the next exposure time, for this we need the
		// mean of the pixel values
		if (targetmean != 0) {
			double	mnew = median(image);
			double	newexp = exposuretime * scalefactor(targetmean / mnew);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "target mean = %f, "
				"actual mean = %f, current exposure time = %f, "
				"new = %f", targetmean, mnew, exposuretime,
				newexp);
			exposuretime = newexp;
		}

		// now wait to the next
		now = time(NULL);
		time_t	delta = next - now;
		if (delta > 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "sleep for %d seconds",
				delta);
			sleep(delta);
		}
	}

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
