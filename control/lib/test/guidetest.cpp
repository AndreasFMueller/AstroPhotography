/*
 * guidetest.cpp -- simulates guiging using the Guider class and the
 *                  simulator driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <AstroImage.h>
#include <AstroGuiding.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroLocator.h>
#include <sstream>
#include <includes.h>

using namespace astro::image;
using namespace astro::guiding;
using namespace astro::image::transform;
using namespace astro::module;
using namespace astro::device;
using namespace astro::camera;

namespace astro {

/**
 * \brief Usage information for the program
 */
void	usage(const char *progname) {
	std::cout << "usage: " << progname;
	std::cout << "[ -dhC ] [ -k range ] [ -s star ] [ -r rectangle ] [ -c calibration ]";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d              increase debug level" << std::endl;
	std::cout << " -h              display this help message and exit";
	std::cout << std::endl;
	std::cout << " -C              perform calibration, without this option," << std::endl;
	std::cout << "                 the calibration from option -c is used";
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
	std::cout << " -c calibration  specify calibration in the format";
	std::cout << std::endl;
	std::cout << "                 [a_11,a_12,a_13;a_21,a_22,a_23]";
	std::cout << std::endl;
	std::cout << " -T guidetime    time during which to perform guiding";
	std::cout << std::endl;
}

/**
 * \brief Default simulator calibration coefficients
 */
static double	defaultcalibration[6] = {
	0.61237243569579452454, -0.50000000000000000000, 0,
	0.35355339059327376220,  0.86602540378443864676, 0
};

/**
 * \brief Main function for the guidetest program
 */
int	guidetest_main(int argc, char *argv[]) {
	debugthreads = 1;
	debugtimeprecision = 3;
	// parse command line
	int	c;
	unsigned int	k = 5;
	ImageRectangle	rectangle;
	ImagePoint	star;
	GuiderCalibration	calibration(defaultcalibration);
	bool	docalibrate = false;
	char	*imagedir = NULL;
	double	temperature = 0;
	double	guidetime = 600;
	while (EOF != (c = getopt(argc, argv, "dk:r:s:h?c:Ci:t:T:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
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
		case 'c':
			std::istringstream(optarg) >> calibration;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "parsed calibration: %s",
				calibration.toString().c_str());
			break;
		case 'C':
			docalibrate = true;
			break;
		case 'i':
			imagedir = optarg;
			break;
		case 't':
			temperature = atof(optarg);
			if (temperature < 0) {
				throw std::runtime_error("temperature must be absolute");
			}
			break;
		case 'T':
			guidetime = atof(optarg);
			break;
		}

	// initialize the random number generator
	srandom(0);

	// load the simulator module from the repository
	Repository	repository;
	ModulePtr	module = repository.getModule("simulator");
	DeviceLocatorPtr	devicelocator = module->getDeviceLocator();

	// now get the simulator camera
	CameraPtr	camera
		= devicelocator->getCamera("camera:simulator/camera");
	CcdPtr	ccd = camera->getCcd(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera: %s, ccd: %s",
		camera->name().name().c_str(),
		ccd->name().name().c_str());

	// if the temperature is set, get the cooler and wait for the
	// chip to cool down
	if (temperature > 0) {
		CoolerPtr	cooler = devicelocator->getCooler("cooler:simulator/cooler");
		cooler->setTemperature(temperature);
		cooler->setOn(true);
		cooler->wait(1000);
	}

	// get the focuser and ensure the camera is actually in focus
	FocuserPtr	focuser
		= devicelocator->getFocuser("focuser:simulator/focuser");
	focuser->moveto(32768);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera focused");

	// if the star an the rectangle are uninitialized, we now initalize
	// them to some reasonable values
	if (rectangle.isEmpty()) {
		rectangle = ccd->getInfo().getFrame();
	}
	if (star.isZero()) {
		star = rectangle.center();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using rectangle %s, star %s",
		rectangle.toString().c_str(), star.toString().c_str());

	// get the guider port
	GuiderPortPtr	guiderport = camera->getGuiderPort();

	// create a Guider object
	Guider	guider(camera, ccd, guiderport);

	// if we have an image directory configuration, we add the 
	// callback
	if (imagedir) {
		guider.newimagecallback = callback::CallbackPtr(
			new callback::SaveImageCallback(imagedir));
	}

	// create a suitable tracker
	ImageRectangle	trackerrectangle(rectangle.size());
	ImagePoint	trackerstar(star - rectangle.origin());
	StarTracker	*startracker = new StarTracker(trackerstar, trackerrectangle, k);
	TrackerPtr	tracker(startracker);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker constructed");

	// set the rectangle
	guider.exposure().frame = rectangle;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure rectangle: %s",
		guider.exposure().frame.toString().c_str());

	// calibrate the guider. This can either be done using the supplied
	// calibration data, or by calling the calibration process
	if (docalibrate) {
		guider.startCalibration(tracker);
		if (!guider.waitCalibration(40 * 9)) {
			std::string	msg = stringprintf("tracker failed to calibrate");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "use configured calibration");
		guider.calibration(calibration);
	}

	// now simulate tracking
	guider.startGuiding(tracker, 10);
	sleep(guidetime);
	guider.stopGuiding();

	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::guidetest_main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "Error in guidetest: " << x.what() << std::endl;
	}
}
