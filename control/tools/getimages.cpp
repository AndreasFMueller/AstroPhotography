/**
 * getimages.cpp -- tool to retrieve a sequence of images from a camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <debug.h>
#include <stdexcept>
#include <Format.h>
#include <iostream>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroIO.h>

using namespace astro;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::image;
using namespace astro::io;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d             increase debug level" << std::endl;
	std::cout << " -?             display this help message and exit"
		<< std::endl;
	std::cout << " -n nImages     number of images to capture"
		<< std::endl;
	std::cout << " -e exptime     exposure time"
		<< std::endl;
	std::cout << " -p prefix      prefix of captured image files"
		<< std::endl;
	std::cout << " -o outputdir      outputdir directory" << std::endl;
	std::cout << " -m modulename  driver modue name, type of the camera"
		<< std::endl;
	std::cout << " -C cameraid    camera number (default 0)"
		<< std::endl;
	std::cout << " -c ccdid       id of the CCD to use (default 0)"
		<< std::endl;
	std::cout << " -w width       width of image rectangle"
		<< std::endl;
	std::cout << " -h height      height of image rectangle"
		<< std::endl;
	std::cout << " -x xoffset     horizontal offset of image rectangle"
		<< std::endl;
	std::cout << " -y yoffset     vertical offset of image rectangle"
		<< std::endl;
	std::cout << " -l             list only, lists the devices"
		<< std::endl;
}

int	main(int argc, char *argv[]) {
	int	c;
	unsigned int	nImages = 1;
	unsigned int	cameranumber = 0;
	unsigned int	ccdid = 0;
	unsigned int	xoffset = 0;
	unsigned int	yoffset = 0;
	unsigned int	width = 0;
	unsigned int	height = 0;
	float	exposuretime = 0.01;
	const char	*outputdir = ".";
	const char	*prefix = "test";
	const char	*cameratype = "uvc";
	bool	listonly = false;

	// parse the command line
	while (EOF != (c = getopt(argc, argv, "dc:C:e:ln:p:o:m:h:w:x:y:?")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'n':
			nImages = atoi(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'p':
			prefix = optarg;
			break;
		case 'o':
			outputdir = optarg;
			break;
		case 'm':
			cameratype = optarg;
			break;
		case 'C':
			cameranumber = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'l':
			listonly = true;
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		case 'x':
			xoffset = atoi(optarg);
			break;
		case 'y':
			yoffset = atoi(optarg);
			break;
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}

	// load the camera driver library
	Repository	repository;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "recovering module '%s'", cameratype);
	ModulePtr	module = repository.getModule(cameratype);
	module->open();

	// get the camera
	CameraLocatorPtr	locator = module->getCameraLocator();
	std::vector<std::string>	cameras = locator->getCameralist();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "have found %d cameras", cameras.size());
	if (0 == cameras.size()) {
		std::cerr << "no cameras found" << std::endl;
		return EXIT_FAILURE;
	}
	if (listonly) {
		int	counter = 0;
		// list the cameras available from this locator
		std::vector<std::string>::const_iterator	i;
		for (i = cameras.begin(); i != cameras.end(); i++) {
			std::cout << cameratype << "[" <<  counter++ << "]"
				<< ": " << *i << std::endl;
		}
		return EXIT_SUCCESS;
	}
	if (cameranumber >= cameras.size()) {
		std::string	msg = stringprintf("camera %d out of range",
			cameranumber);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s\n", msg.c_str());
		throw std::range_error(msg);
	}
	std::string	cameraname = cameras[cameranumber];
	CameraPtr	camera = locator->getCamera(cameraname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera loaded: %s", cameraname.c_str());

	// get a CCD
	CcdPtr	ccd = camera->getCcd(ccdid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd: %s",
		ccd->getInfo().toString().c_str());

	// create the image rectangle
	if (width == 0) {
		width = ccd->getInfo().size.width;
	}
	if (height == 0) {
		height = ccd->getInfo().size.height;
	}
	ImageRectangle	imagerectangle = ccd->getInfo().clipRectangle(
		ImageRectangle(ImagePoint(xoffset, yoffset),
			ImageSize(width, height)));

	// prepare an exposure object
	Exposure	exposure(imagerectangle, exposuretime);

	// start the exposure
	ccd->startExposure(exposure);

	// read all images
	ImageSequence	images = ccd->getImageSequence(nImages);

	// write the images to a file
	ImageSequence::const_iterator	imageptr;
	int	counter = 0;
	for (imageptr = images.begin(); imageptr != images.end(); imageptr++) {
		std::string	filename = stringprintf("%s/%s%03d.fits",
			outputdir, prefix, counter++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing image %s",
			filename.c_str());
		unlink(filename.c_str());
		FITSout	out(filename);
		out.write(*imageptr);
	}

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::string	msg = stringprintf("%s terminated: %s",
			argv[0], x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s\n", msg.c_str());
		std::cerr << msg << std::endl;
	}
}
