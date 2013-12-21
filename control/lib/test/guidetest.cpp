/*
 * guidetest.cpp -- simulates guiging using the Guider class and the
 *                  Simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <AstroImage.h>
#include <AstroGuiding.h>
#include <Simulator.h>
#include <AstroDebug.h>
#include <sstream>

using namespace astro::image;
using namespace astro::guiding;
using namespace astro::camera::sim;
using namespace astro::image::transform;

namespace astro {

void	usage(const char *progname) {
	std::cout << "usage: " << progname;
	std::cout << "[ -dh ] [ -k range ] [ -s star ] [ -r rectangle ] ";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d              increase debug level" << std::endl;
	std::cout << " -h              display this help message and exit";
	std::cout << std::endl;
	std::cout << " -k range        number indicating the range of pixels to";
	std::cout << std::endl;
	std::cout << "                 inspect when computing the displacement";
	std::cout << std::endl;
	std::cout << " -s star         pixel coordinates of star" << std::endl;
	std::cout << " -r rectangle    image rectangle from camera image,";
	std::cout << std::endl;
	std::cout << "                 must be contained in CCD area";
	std::cout << std::endl;
}

int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	unsigned int	k = 5;
	ImageRectangle	rectangle;
	ImagePoint	star;
	while (EOF != (c = getopt(argc, argv, "dk:r:s:h")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'k':
			k = atoi(optarg);
			break;
		case 's':
			std::istringstream(optarg) >> star;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"star option '%s' parsed as %s", optarg,
				star.toString().c_str());
			break;
		case 'r':
			std::istringstream(optarg) >> rectangle;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"rectangle option '%s' parsed as %s", optarg,
				rectangle.toString().c_str());
			break;
		}

	// load the simulator camera
	SimCamera	*camera = new SimCamera();
	if (NULL == camera) {
		throw std::runtime_error("no simulator camera found");
	}
	CameraPtr	cameraptr = CameraPtr(camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find simulator camera");
	CcdPtr	ccd = cameraptr->getCcd(0);
	if (!ccd) {
		throw std::runtime_error("no simulator CCD");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find simulator camera");

	// if the star an the rectangle are uninitialized, we now initalize
	// them to some reasonable values
	if (rectangle.isEmpty()) {
		rectangle = ccd->getInfo().getFrame();
	}
	if (star.isZero()) {
		star = rectangle.center();
	}

	// get the guider port
	GuiderPortPtr	guiderport = cameraptr->getGuiderPort();

	// create a Guider object
	Guider	guider(cameraptr, ccd, guiderport);

	// create a suitable tracker
	StarTracker	*startracker = new StarTracker(star, rectangle, k);
	TrackerPtr	tracker(startracker);

	// calibrate the 
	guider.startCalibration(tracker);
	if (!guider.waitCalibration(60)) {
		std::string	msg = stringprintf("tracker failed to calibrate");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// now simulate tracking
	guider.startGuiding(tracker);
	sleep(60);
	guider.stopGuiding();

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "Error in guidetest: " << x.what() << std::endl;
	}
}
