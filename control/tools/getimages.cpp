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

int	main(int argc, char *argv[]) {
	int	c;
	unsigned int	nImages = 1;
	unsigned int	cameranumber = 0;
	unsigned int	ccdid = 0;
	float	exposuretime = 0.01;
	const char	*target = ".";
	const char	*prefix = "test";
	const char	*cameratype = "uvc";
	bool	listonly = false;

	// parse the command line
	while (EOF != (c = getopt(argc, argv, "dc:C:e:ln:p:t:T:")))
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
		case 't':
			target = optarg;
			break;
		case 'T':
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
		}

	// load the camera driver library
	Repository	repository;
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

	// prepare an exposure object
	Exposure	exposure(ccd->getInfo().size, exposuretime);

	// start the exposure
	ccd->startExposure(exposure);

	// read all images
	ImageSequence	images = ccd->getImageSequence(nImages);

	// write the images to a file
	ImageSequence::const_iterator	imageptr;
	int	counter = 0;
	for (imageptr = images.begin(); imageptr != images.end(); imageptr++) {
		std::string	filename = stringprintf("%s/%s%03d.fits",
			target, prefix, counter++);
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
