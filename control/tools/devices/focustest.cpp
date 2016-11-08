/*
 * focustest.cpp -- Measure the effects of the focuser
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include <includes.h>
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroFilterfunc.h>
#include <AstroGuiding.h>
#include <AstroIO.h>
#include <AstroCalibration.h>
#include <AstroInterpolation.h>
#if ENABLE_CORBA
#include <OrbSingleton.h>
#endif /* ENABLE_CORBA */
#include <AstroUtils.h>

using namespace astro;
using namespace astro::camera;
using namespace astro::image;
using namespace astro::module;
using namespace astro::device;
using namespace astro::image::filter;
using namespace astro::guiding;
using namespace astro::io;
using namespace astro::calibration;
using namespace astro::interpolation;

namespace astro {
namespace app {
namespace focustest {

int	main(int argc, char *argv[]) {

	int	c;
	const char	*cameramodulename = "simulator";
	int	cameraid = 0;
	int	ccdid = 0;
	double	exposuretime = 1.;
	const char	*focusermodulename = "module:simulator";
	const char	*focusername = "focuser:simulator/focuser";
	unsigned short	begin = 0;
	unsigned short	end = std::numeric_limits<unsigned short>::max();
	unsigned short	step = 1024;
	unsigned short	position = 0;
	ImagePtr	darkimage;

	// get the ORB
#if ENABLE_CORBA
	Astro::OrbSingleton	orb(argc, argv);
#endif /* ENABLE_CORBA */
	debugtimeprecision = 3;

	while (EOF != (c = getopt(argc, argv, "dm:C:c:F:f:b:e:s:E:p:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'm':
			cameramodulename = optarg;
			break;
		case 'C':
			cameraid = atoi(optarg);
			break;
		case 'c':
			ccdid = atoi(optarg);
			break;
		case 'F':
			focusermodulename = optarg;
			break;
		case 'f':
			focusername = optarg;
			break;
		case 'p':
			position = atoi(optarg);
			break;
		case 'b':
			begin = atoi(optarg);
			break;
		case 'e':
			end = atoi(optarg);
			break;
		case 's':
			step = atoi(optarg);
			break;
		case 'E':
			exposuretime = atof(optarg);
			break;
		case 'D':
			{
			FITSin	in(optarg);
			darkimage = in.read();
			}
			break;
		default:
			throw std::runtime_error("unknown option");
		}

	// get the repository
	Repository	repository;
	ModulePtr	cameramodule = repository.getModule(cameramodulename);
	cameramodule->open();

	// get the Camera
	DeviceLocatorPtr	cameralocator
		= cameramodule->getDeviceLocator();
	std::vector<std::string>	cameranames
		= cameralocator->getDevicelist();
	std::string	cameraname = cameranames[cameraid];
	CameraPtr	camera = cameralocator->getCamera(cameraname);

	// Get the CCD
	CcdPtr	ccd = camera->getCcd(ccdid);

	// Get the focuser
	DeviceLocatorPtr	focuserlocator = cameralocator;
	if (std::string(focusermodulename) != std::string(cameramodulename)) {
		ModulePtr	focusermodule
			= repository.getModule(focusermodulename);
		focuserlocator = focusermodule->getDeviceLocator();
	}
	FocuserPtr	focuser = focuserlocator->getFocuser(focusername);

	// move to the prescribed position
	if (position != 0) {
		focuser->moveto(position, 60);
	}

	// prepare the Exposure object we will later use for exposing
	Exposure	exposure;
	exposure.exposuretime(exposuretime);

	// now we need to create a dark image, except if we already have
	// one, from the D option
	if (darkimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "already have a dark image");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting dark image");
		DarkFrameProcess	darkproc(ccd);
		darkproc.setExposuretime(exposuretime);
		darkproc.setNimages(5);
		darkimage = darkproc.get();
	}

	// create a dark corrector from this dark
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating dark corrector");
	DarkCorrector	darkcorrector(darkimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating interpolator");
	Interpolator	interpolator(darkimage);

	// get an image
	ccd->startExposure(exposure);
	ccd->wait();
	ImagePtr	image = ccd->getImage();
	darkcorrector(image);
	interpolator(image);
	FITSout	out("focus.fits");
	out.setPrecious(false);
	out.write(image);

	// get the brightest point in the image
	Point	centerd = findstar(image, image->getFrame(), 8);
	ImagePoint	center((int)lround(centerd.x()), (int)lround(centerd.y()));
	double	radius = 20;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "brightest star at %s",
		center.toString().c_str());

	// take images for all focuser positions, and report the focus FOM
	std::cout << "pos,fom,fwhm" << std::endl;
	for (unsigned short position = begin; position < end; position += step){
		// move to this position
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new position: %hu", position);
		if (!focuser->moveto(position)) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot move focuser to %hu", position);
			return EXIT_FAILURE;
		}
		std::cout << position;

		// now take an image
		debug(LOG_DEBUG, DEBUG_LOG, 0, "take new image");
		ccd->startExposure(exposure);
		ccd->wait();
		ImagePtr	image = ccd->getImage();

		// calibrate the image
		darkcorrector(image);
		interpolator(image);

		// analyze image, focusFOM first
		double	fom = focusFOM(image, false);
		std::cout << "," << fom;

		// compute focusFWHM
		double	fwhm = focusFWHM(image, center, radius);
		std::cout << "," << fwhm;

		std::cout << std::endl;
	}

	return EXIT_SUCCESS;
}

} // namespace focustest
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::focustest::main>(argc, argv);
}
