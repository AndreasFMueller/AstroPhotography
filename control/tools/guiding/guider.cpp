/*
 * guider.cpp -- simple guider program
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <stdexcept>
#include <iostream>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <AstroLoader.h>
#include <AstroGuiding.h>
#include <AstroCallback.h>
#include <AstroIO.h>
#include <AstroUtils.h>
#include <typeinfo>

using namespace astro;
using namespace astro::camera;
using namespace astro::device;
using namespace astro::module;
using namespace astro::guiding;
using namespace astro::callback;
using namespace astro::io;

namespace astro {
namespace app {
namespace guider {

class NewImageCallback : public Callback {
	FITSdirectory	directory;
public:
	NewImageCallback(const std::string& path) : directory(path) { }
	virtual CallbackDataPtr	operator()(CallbackDataPtr data);
};

CallbackDataPtr	NewImageCallback::operator()(CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback received");
	ImageCallbackData	*datap
		= dynamic_cast<ImageCallbackData *>(&*data);
	if (datap == NULL) {
		debug(LOG_ERR, DEBUG_LOG, 0, "argument not of correct type");
	}
	// retrieve image 
	ImagePtr	image = datap->image();
	// add it to the directory
	directory.add(image);

	// return the same data
	return data;
}

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename() << " [ options ] cmd time { cmd time ... }" << std::endl;
	std::cout << std::endl;
	std::cout << "commands are: R+, R-, D+, D-. They modify the speed of "
		"the telescope drive," << std::endl;
	std::cout << "in right ascension and declination for a given time "
		"in ms." << std::endl;
	std::cout << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << " -d,--debug              enable debug mode" << std::endl;
	std::cout << " -m,--module=<modulde>   load driver module named <module>"
		<< std::endl;
	std::cout << " -C,--camera=<cameraid>  select camera number <cameraid>"
		<< std::endl;
	std::cout << " -e,--exposure=<time>    exposure time" << std::endl;
	std::cout << " -k,--width=<k>          half side length of square of "
		"pixels to include in" << std::endl;
	std::cout << "                         centroid computation"
		<< std::endl;
	std::cout << " -x,--x=<x>              approx. x coordinate of guide "
		"star" << std::endl;
	std::cout << " -y,--y=<y>              approx. y coordinate of guide "
		"star" << std::endl;
	std::cout << " -r,--radius=<radius>    search radius for guide star"
		<< std::endl;
	std::cout << " -p,--path=<path>        path where images should be "
		"written" << std::endl;
}

static struct option	longopts[] = {
{ "debug",	no_argument,		NULL,	'd' }, /* 0 */
{ "module",	required_argument,	NULL,	'm' }, /* 1 */
{ "camera",	required_argument,	NULL,	'C' }, /* 2 */
{ "exposure",	required_argument,	NULL,	'e' }, /* 3 */
{ "instrument",	required_argument,	NULL,	'i' }, /* 3 */
{ "width",	required_argument,	NULL,	'k' }, /* 4 */
{ "x",		required_argument,	NULL,	'x' }, /* 5 */
{ "y",		required_argument,	NULL,	'y' }, /* 6 */
{ "radius",	required_argument,	NULL,	'r' }, /* 7 */
{ "path",	required_argument,	NULL,	'p' }, /* 8 */
{ NULL,		0,			NULL,	 0  }, /* 9 */
};

int	main(int argc, char *argv[]) {
	// parse command line arguments
	int	c;
	int	longindex;
	unsigned int	cameraid = 0;
	unsigned int	ccdid = 0;
	const char	*modulename = "uvc";
	double	exposuretime = 1;
	unsigned int	k = 5;
	int	x = -1;
	int	y = -1;
	int	r = 32;
	const char	*path = NULL;
	std::string	instrument;
	while (EOF != (c = getopt_long(argc, argv, "dm:C:c:e:i:k:x:y:r:p:h?",
		longopts, &longindex)))
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
		case 'i':
			instrument = std::string(optarg);
			break;
		case 'k':
			k = atoi(optarg);
			break;
		case 'x':
			x = atoi(optarg);
			break;
		case 'y':
			y = atoi(optarg);
			break;
		case 'r':
			r = atoi(optarg);
			break;
		case 'p':
			path = optarg;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			usage(argv[0]);
			return EXIT_FAILURE;
		}

	// get the driver module
	ModuleRepositoryPtr	repository = getModuleRepository();
	ModulePtr	module = repository->getModule(modulename);
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
 
	// compute the point where we should look for the guide star
	if (x < 0) {
		x = ccd->getInfo().size().width() / 2;
	}
	if (y < 0) {
		y = ccd->getInfo().size().height() / 2;
	}

	ImageRectangle	starwindow(ImagePoint(x - r, y - r),
		ImageSize(2 * r, 2 * r));

	// get the Guider for the camera
	GuidePortPtr	guideport = camera->getGuidePort();
	
	// now interpret the remaining arguments as control commands
	if (optind < argc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "test guiding commands");
		while (optind < argc) {
			char	*direction = argv[optind++];
			double	duration = atoi(argv[optind++]) / 1000.;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"found command %s for %.3fs",
				direction, duration);
			if (0 == strcmp(direction, "D-")) {
				guideport->activate(0, 0, 0, duration);
			}
			if (0 == strcmp(direction, "D+")) {
				guideport->activate(0, 0, duration, 0);
			}
			if (0 == strcmp(direction, "R-")) {
				guideport->activate(0, duration, 0, 0);
			}
			if (0 == strcmp(direction, "R+")) {
				guideport->activate(duration, 0, 0, 0);
			}
			usleep(1000000 * duration);
			printf(""); fflush(stdout);
		}
		sleep(2);
		return EXIT_SUCCESS;
	}

	// create a guider
	Guider	guider(instrument, ccd, guideport, NULL);

	// if the path is set, we also install a callback
	if (path) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "installing callback to write "
			"images to directory %s", path);
		NewImageCallback	*callback = new NewImageCallback(path);
		guider.addImageCallback(CallbackPtr(callback));
	}

	// get a first image from the ccd, which we use to find the brightest
	// star
	Exposure	exposure(ccd->getInfo().getFrame(), exposuretime);
	exposure.shutter(Shutter::OPEN);
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starwindow: %s", starwindow.toString().c_str());
	Point	guidestar = findstar(image, starwindow);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found guide star at %s",
		guidestar.toString().c_str());

	// create a tracker based on this guide star 
	StarTracker	*startracker = new StarTracker(guidestar,
		ccd->getInfo().getFrame());
	TrackerPtr	tracker(startracker);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker created");

	// now perform the calibration
	guider.exposure(exposure);
	guider.startCalibration(GP, tracker);
	double	timeout = 120;
	if (!guider.waitCalibration(timeout)) {
		std::string	msg = stringprintf("tracker failed to calibrate");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// now track for 2 hours
	guider.startGuiding(tracker, 10);
	sleep(7200);
	guider.stopGuiding();

	return EXIT_SUCCESS;
}


} // namespace guider
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::guider::main>(argc, argv);
}
