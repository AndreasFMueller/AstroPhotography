/*
 * focus.cpp -- command line focus utility
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <iostream>
#include <AstroFormat.h>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <AstroFilter.h>
#include <AstroFilterfunc.h>
#include <AstroIO.h>
#include <stacktrace.h>
#include <includes.h>

using namespace astro;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::io;

namespace astro {

int	main(int argc, char *argv[]) {
	int	c;
	double	exposuretime = 0.1;
	unsigned int	cameraid = 0;
	unsigned int	ccdid = 0;
	int	length = 512;
	std::string	cameratype("uvc");

	while (EOF != (c = getopt(argc, argv, "de:m:c:C:l:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'm':
			cameratype = std::string(optarg);
			break;
		case 'C':
			cameraid = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'l':
			length = atoi(optarg);
			break;
		}

	// get the camera
	Repository	repository;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "loading module %s",
		cameratype.c_str());
	ModulePtr	module = repository.getModule(cameratype);
	module->open();


	// get the camera
	DeviceLocatorPtr	locator = module->getDeviceLocator();
	std::vector<std::string>	cameras = locator->getDevicelist();
	if (0 == cameras.size()) {
		std::cerr << "no cameras found" << std::endl;
		return EXIT_FAILURE;
	}
	if (cameraid >= cameras.size()) {
		std::string	msg = stringprintf("camera %d out of range",
			cameraid);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s\n", msg.c_str());
		throw std::range_error(msg);
	}
	std::string	cameraname = cameras[cameraid];
	CameraPtr	camera = locator->getCamera(cameraname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera loaded: %s", cameraname.c_str());

	// get the ccd
	CcdPtr	ccd = camera->getCcd(ccdid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a ccd: %s",
		ccd->getInfo().toString().c_str());

	// get a centerd length x length frame
	ImageSize	framesize(length, length);
	ImageRectangle	frame = ccd->getInfo().centeredRectangle(framesize);
	Exposure	exposure(frame, exposuretime);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure prepared: %s",
		exposure.toString().c_str());

	// retrieve an image
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();

	// write image
	unlink("test.fits");
	FITSout	out("test.fits");
	out.write(image);

	// apply a mask to keep the border out
	CircleFunction	circle(ImagePoint(length/2, length/2), length/2, 0.8);
	mask(circle, image);
	unlink("masked.fits");
	FITSout	maskout("masked.fits");
	maskout.write(image);

#if 0
	// compute the FOM
	double	fom = focusFOM(image, true,
		Subgrid(ImagePoint(1, 0), ImageSize(1, 1)));
	std::cout << "FOM: " << fom << std::endl;
#endif


	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	signal(SIGSEGV, stderr_stacktrace);
	try {
		return astro::main(argc, argv);
	} catch (const std::exception& x) {
		std::cerr << "focus terminated by ";
		std::cerr << astro::demangle(typeid(x).name());
		std::cerr << ": " << x.what() << std::endl;
	} catch (...) {
		std::cerr << "terminated by unknown exception" << std::endl;
	}
	return EXIT_FAILURE;
}
