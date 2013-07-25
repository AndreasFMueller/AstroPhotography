/*
 * guider.cpp -- simple guider program
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <AstroLoader.h>
#include <AstroGuiding.h>

using namespace astro::camera;
using namespace astro::device;
using namespace astro::module;
using namespace astro::guiding;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname << " [ -d ] [ -m drivermodule ] [ -C cameraid ] cmd time { cmd time ... }" << std::endl;
	std::cout << "commands are: R+, R-, D+, D-. They modify the speed of "
		"the telescope drive," << std::endl;
	std::cout << "in right ascension and declination for a given time "
		"in ms." << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d             enable debug mode" << std::endl;
	std::cout << " -m <modulde>   load driver module named <module>"
		<< std::endl;
	std::cout << " -C <cameraid>  select camera number <cameraid>"
		<< std::endl;
}

int	main(int argc, char *argv[]) {
	// parse command line arguments
	int	c;
	unsigned int	cameraid = 0;
	unsigned int	ccdid = 0;
	const char	*modulename = "uvc";
	double	exposuretime = 1;
	unsigned int	k = 5;
	while (EOF != (c = getopt(argc, argv, "dm:C:c:e:k:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'm':
			modulename = optarg;
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
		case 'k':
			k = atoi(optarg);
			break;
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}

	// get the driver module
	Repository	repository;
	ModulePtr	module = repository.getModule(modulename);
	module->open();

	// get the camera
	DeviceLocatorPtr	locator = module->getDeviceLocator();
	std::vector<std::string>	cameras = locator->getDevicelist();
	if (cameraid >= cameras.size()) {
		std::cerr << "cameraid exceeds number of cameras" << std::endl;
		throw std::runtime_error("bad cameraid");
	}
	CameraPtr	camera = locator->getCamera(cameras[cameraid]);
	CcdPtr	ccd = camera->getCcd(ccdid);

	// get the Guider for the camera
	GuiderPortPtr	guiderport = camera->getGuiderPort();
	
	// now interpret the remaining arguments as control commands
	if (optind < argc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "test guiding commands");
		while (optind < argc) {
			char	*direction = argv[optind++];
			double	duration = atoi(argv[optind++]) / 1000.;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found command %s for %.3fs",
				direction, duration);
			if (0 == strcmp(direction, "D-")) {
				guiderport->activate(0, 0, 0, duration);
			}
			if (0 == strcmp(direction, "D+")) {
				guiderport->activate(0, 0, duration, 0);
			}
			if (0 == strcmp(direction, "R-")) {
				guiderport->activate(0, duration, 0, 0);
			}
			if (0 == strcmp(direction, "R+")) {
				guiderport->activate(duration, 0, 0, 0);
			}
			usleep(1000000 * duration);
			printf(""); fflush(stdout);
		}
		sleep(2);
		return EXIT_SUCCESS;
	}

	// create a guider
	Guider	guider(guiderport, ccd);

	// get a first image from the ccd, which we use to find the brightest
	// star
	Exposure	exposure(ccd->getInfo().getFrame(), exposuretime);
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();
	Point	guidestar = findstar(image, ccd->getInfo().getFrame(), k);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found guide star at %s",
		guidestar.toString().c_str());

	// create a tracker based on this guide star 
	StarTracker	*startracker = new StarTracker(guidestar,
		ccd->getInfo().getFrame(), k);
	TrackerPtr	tracker(startracker);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker created");

	// now perform the calibration
	guider.setExposure(exposure);
	if (!guider.calibrate(tracker)) {
		std::string	msg = stringprintf("tracker failed to calibrate");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// now track for 10 minutes
	guider.start(tracker);
	sleep(600);
	guider.stop();

	return EXIT_SUCCESS;
}


} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "guider terminated by exception: " << x.what()
			<< std::endl;
	}
}
