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
	std::cout << "  -n images    number of images" << std::endl;
	std::cout << "  -o outdir    directory where files should be placed"
		<< std::endl;
	std::cout << "  -E mean      attempt to vary the exposure time in such a way that" << std::endl;
	std::cout << "               that the mean pixel value stays close to mean" << std::endl;
	std::cout << "  -?           display this help message" << std::endl;
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
	double		targetmean = 16000;
	while (EOF != (c = getopt(argc, argv, "dw:x:y:w:h:o:C:c:n:e:E:m:?")))
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

	// now initialize exposure computation loop
	unsigned int	counter = 0;
	while (counter++ < nImages) {
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
			double	newexp = exposuretime * sqrt(targetmean / mnew);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "target mean = %f, "
				"actual mean = %f, current exposure time = %f, "
				"new = %f", targetmean, mnew, exposuretime, newexp);
			exposuretime = newexp;
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
