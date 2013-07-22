/*
 * guider.cpp -- simple guider program
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <AstroCamera.h>
#include <AstroLoader.h>

using namespace astro::camera;
using namespace astro::module;

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
	const char	*modulename = "uvc";
	while (EOF != (c = getopt(argc, argv, "dm:C:")))
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
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}

	// get the driver module
	Repository	repository;
	ModulePtr	module = repository.getModule(modulename);
	module->open();

	// get the camera
	CameraLocatorPtr	locator = module->getCameraLocator();
	std::vector<std::string>	cameras = locator->getCameralist();
	if (cameraid >= cameras.size()) {
		std::cerr << "cameraid exceeds number of cameras" << std::endl;
		throw std::runtime_error("bad cameraid");
	}
	CameraPtr	camera = locator->getCamera(cameras[cameraid]);

	// get the Guider for the camera
	GuiderPortPtr	guider = camera->getGuiderPort();

	// now interpret the remaining arguments as control commands
	while (optind < argc) {
		char	*direction = argv[optind++];
		double	duration = atoi(argv[optind++]) / 1000.;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found command %s for %.3fs",
			direction, duration);
		if (0 == strcmp(direction, "D-")) {
			guider->activate(0, 0, 0, duration);
		}
		if (0 == strcmp(direction, "D+")) {
			guider->activate(0, 0, duration, 0);
		}
		if (0 == strcmp(direction, "R-")) {
			guider->activate(0, duration, 0, 0);
		}
		if (0 == strcmp(direction, "R+")) {
			guider->activate(duration, 0, 0, 0);
		}
		usleep(1000000 * duration);
		printf(""); fflush(stdout);
	}
	sleep(3);

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
