/*
 * getimages.cpp -- tool to retrieve a sequence of images from a camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <AstroFormat.h>
#include <iostream>
#include <sstream>
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <AstroIO.h>
#include <OrbSingleton.h>

using namespace astro;
using namespace astro::module;
using namespace astro::camera;
using namespace astro::device;
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
	std::cout << " -t temp        cool the CCD to temperature <temp> in decrees Celsius"
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
	bool	dark = false;
	double	temperature = -1;
	unsigned short	focus = 32768;
	const char	*focuser = NULL;

	// initialize the orb in case we want to use the net module
	Astro::OrbSingleton	orb(argc, argv);
	debugtimeprecision = 3;
	int	binning = 1;

	// parse the command line
	while (EOF != (c = getopt(argc, argv, "b:dc:C:e:ln:p:o:m:h:w:x:y:?Dt:f:F:")))
		switch (c) {
		case 'b':
			binning = atoi(optarg);
			if ((binning > 4) || (binning < 1)) {
				throw std::runtime_error("illegal binning mode");
			}
			break;
		case 'D':
			dark = true;
			break;
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
		case 't':
			temperature = atof(optarg) + 273.1;
			break;
		case 'f':
			focus = atoi(optarg);
			break;
		case 'F':
			focuser = optarg;
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
	DeviceLocatorPtr	locator = module->getDeviceLocator();
	std::vector<std::string>	cameras = locator->getDevicelist();
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "requesting camera %s",
		cameraname.c_str());
	CameraPtr	camera = locator->getCamera(cameraname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera loaded: %s", cameraname.c_str());

	// get a CCD
	CcdPtr	ccd = camera->getCcd(ccdid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a ccd: %s",
		ccd->getInfo().toString().c_str());

	// create the image rectangle
	if (width == 0) {
		width = ccd->getInfo().size().width();
	}
	if (height == 0) {
		height = ccd->getInfo().size().height();
	}
	ImageRectangle	imagerectangle = ccd->getInfo().clipRectangle(
		ImageRectangle(ImagePoint(xoffset, yoffset),
			ImageSize(width, height)));

	// if the focuser is specified, we try to get it and then set
	// the focus value
	if (focuser) {
		std::string	name(focuser);
		FocuserPtr	f = locator->getFocuser(std::string(focuser));
		f->set(focus);
		while (f->current() != focus) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"current = %hu, focus = %hu",
				f->current(), focus);
			usleep(100000);
		}
	}

	// if the temperature is set, and the ccd has a cooler, lets
	// start the cooler
	bool	usecooler = (ccd->hasCooler() && (temperature > 0));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler: %s, temperature = %f",
		usecooler ? "YES" : "NO", temperature);
	CoolerPtr	cooler;
	if (usecooler) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing the cooler");
		cooler = ccd->getCooler();
		cooler->setTemperature(temperature);
		cooler->setOn(true);
		// wait until the temperature is within 1 degree of the
		// set temperature
		double	delta;
		do {
			sleep(1);
			double	actual = cooler->getActualTemperature();
			delta = fabs(temperature - actual);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"set: %.1f, actual: %1.f, delta: %.1f",
				temperature, actual, delta);
		} while (delta > 1);
	}

	// prepare an exposure object
	Exposure	exposure(imagerectangle, exposuretime);
	exposure.shutter = (dark) ? SHUTTER_CLOSED : SHUTTER_OPEN;
	exposure.mode = Binning(binning, binning);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s",
		exposure.toString().c_str());

	// check whether the remote camera already has an exposed image,
	// in which case we want to cancel it
	if (Exposure::exposed == ccd->exposureStatus()) {
		ccd->cancelExposure();
		while (Exposure::idle != ccd->exposureStatus()) {
			usleep(100000);
		}
	}

	// start the exposure
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");
	ccd->startExposure(exposure);
	usleep(1000000 * exposuretime);

	// read all images
	ImageSequence	images = ccd->getImageSequence(nImages);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieved %d images", images.size());

	// turn of the cooler to save energy
	if (usecooler) {
		cooler->setOn(false);
	}

	// write the images to a file
	ImageSequence::const_iterator	imageptr;
	int	counter = 0;
	for (imageptr = images.begin(); imageptr != images.end(); imageptr++) {
		std::string	filename = stringprintf("%s/%s%03d.fits",
			outputdir, prefix, counter++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing image %s of size %s",
			filename.c_str(), (*imageptr)->size().toString().c_str());
		if (debuglevel >= LOG_DEBUG) {
			std::ostringstream	out;
			out << **imageptr;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image info: %s",
				out.str().c_str());
		}
		unlink(filename.c_str());
		FITSout	out(filename);
		out.write(*imageptr);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d images written", counter);

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
