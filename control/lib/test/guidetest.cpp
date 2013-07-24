/*
 * guidetest.cpp -- simulates guiging using the Guider class and the
 *                  Simulator camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <AstroGuiding.h>
#include <Simulator.h>
#include <debug.h>

using namespace astro::image;
using namespace astro::guiding;
using namespace astro::camera::sim;
using namespace astro::image::transform;

namespace astro {

int	main(int argc, char *argv[]) {
	// parse command line
	int	c;
	unsigned int	k = 5;
	while (EOF != (c = getopt(argc, argv, "dk:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'k':
			k = atoi(optarg);
			break;
		}

	// load the simulator camera
	SimCamera	*camera = new SimCamera();
	CameraPtr	cameraptr = CameraPtr(camera);
	CcdPtr	ccd = cameraptr->getCcd(0);

	// get the guider port
	GuiderPortPtr	guiderport = cameraptr->getGuiderPort();

	// create a Guider object
	Guider	guider(guiderport, ccd);

	// create a suitable tracker
	StarTracker	*startracker = new StarTracker(Point(320, 240),
		ImageRectangle(ImagePoint(270, 190), ImageSize(100, 100)), k);
	TrackerPtr	tracker(startracker);

	// calibrate the 
	if (!guider.calibrate(tracker)) {
		std::string	msg = stringprintf("tracker failed to calibrate");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// XXX now simulate tracking

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "" << x.what() << std::endl;
	}
}
