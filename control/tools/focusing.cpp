/*
 * focusing.cpp -- perform focusing
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <iostream>
#include <AstroDebug.h>
#include <AstroDevice.h>
#include <AstroLoader.h>
#include <includes.h>
#include <AstroFocus.h>
#include <AstroFilterfunc.h>
#include <AstroFormat.h>
#include <AstroIO.h>

using namespace astro::focusing;

namespace astro {

class FWHMEvaluator : public FocusEvaluator {
	ImagePoint	_center;
	double	_radius;
public:
	FWHMEvaluator(const ImagePoint& center, double radius = 20)
		: _center(center), _radius(radius) { }
	virtual double	operator()(const ImagePtr image) {
		double  fwhm = astro::image::filter::focusFWHM(image, _center, _radius);
		return fwhm;
	}
};

class FWHM2Evaluator : public FocusEvaluator {
	ImagePoint	_center;
	double	_radius;
public:
	FWHM2Evaluator(const ImagePoint& center, double radius = 20)
		: _center(center), _radius(radius) { }
	virtual double	operator()(const ImagePtr image) {
		double  fwhm = astro::image::filter::focusFWHM2(image, _center, _radius);
		return fwhm;
	}
};

class FOMEvaluator : public FocusEvaluator {
public:
	FOMEvaluator() { }
	virtual double	operator()(const ImagePtr image) {
		double  fom = astro::image::filter::focusFOM(image);
		return 1. / fom;
	}
};

class FocusingCallback : public astro::callback::Callback {
	std::string	_prefix;
	int	counter;
public:
	FocusingCallback(const std::string& prefix) : _prefix(prefix) {
		counter = 0;
	}
	astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "executing callback");
		if (!data) {
			return data;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "find argument type");
		astro::callback::ImageCallbackData	*i
			= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
		if (NULL == i) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "not an ImageCallbackData");
			return data;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got an ImageCallbackData");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image size: %s", i->image()->size().toString().c_str());
		std::string	filename
			= _prefix + stringprintf("-%d.fits", counter++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file name: %s",
			filename.c_str());
#if 1
		astro::io::FITSout	outfile(filename);
		outfile.write(i->image());
#endif
		return data;
	}
};

int	main(int argc, char *argv[]) {
	int	c;
	unsigned short	min = 28000;
	unsigned short	max = 36000;
	int	steps = 10;
	DeviceName	ccdname("ccd:simulator/camera/ccd");
	DeviceName	focusername("focuser:simulator/focuser");
	double	exposuretime = 1;
	int	x = -1;
	int	y = -1;
	int	width = -1;
	int	height = -1;
	while (EOF != (c = getopt(argc, argv, "dm:M:C:F:s:e:x:y:w:h:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			debugthreads = 1;
			break;
		case 'm':
			min = atoi(optarg);
			break;
		case 'M':
			max = atoi(optarg);
			break;
		case 'C':
			ccdname = DeviceName(optarg);
			break;
		case 'F':
			focusername = DeviceName(optarg);
			break;
		case 's':
			steps = atoi(optarg);
			break;
		case 'e':
			exposuretime = atof(optarg);
			break;
		case 'x':
			x = atoi(optarg);
			break;
		case 'y':
			y = atoi(optarg);
			break;
		case 'w':
			width = atoi(optarg);
			break;
		case 'h':
			height = atoi(optarg);
			break;
		}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "start focusing process");

	// get the device repository
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);

	// get the ccd
	astro::camera::CcdPtr	ccd = devices.getCcd(ccdname);

	// get missing information from the CCD
	if (x < 0) { x = ccd->getInfo().size().width() / 2; }
	if (y < 0) { y = ccd->getInfo().size().height() / 2; }
	if (width < 0) { width = ccd->getInfo().size().width(); }
	if (height < 0) { height = ccd->getInfo().size().height(); }

	// get the focuser
	astro::camera::FocuserPtr	focuser
		= devices.getFocuser(focusername);
	sleep(2); // to ensure the filter wheel is idle for the simulator

	// prepare the exposure 
	astro::camera::Exposure	exposure;
	exposure.exposuretime = exposuretime;
	exposure.frame = ImageRectangle(
		ImagePoint(x - width / 2, y - height / 2),
		ImageSize(width, height));
	exposure.shutter = astro::camera::SHUTTER_OPEN;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s",
		exposure.toString().c_str());

	// run the focusing process
	Focusing	focusing(ccd, focuser);
	focusing.exposure(exposure);
	focusing.steps(steps);

	//focusing.evaluator(FocusEvaluatorPtr(new FWHMEvaluator(ImagePoint(x, y), 50)));
	focusing.evaluator(FocusEvaluatorPtr(new FWHM2Evaluator(ImagePoint(width/2, height / 2), 50)));
	//focusing.evaluator(FocusEvaluatorPtr(new FOMEvaluator()));

	// install the callback
	astro::callback::CallbackPtr	cbptr = astro::callback::CallbackPtr(
		new FocusingCallback(std::string("fc/image")));
	focusing.callback(cbptr);

	// now start the process
	focusing.start(min, max);

	// wait until focusing is complete
	while (Focusing::FOCUSED != focusing.status()) {
		std::string	statusname;
		switch (focusing.status()) {
		case Focusing::IDLE:
			statusname = "idle";
			break;
		case Focusing::MOVING:
			statusname = "moving";
			break;
		case Focusing::MEASURING:
			statusname = "measuring";
			break;
		case Focusing::FOCUSED:
			statusname = "focused";
			break;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing status: %s",
			statusname.c_str());
		sleep(1);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "focusing process complete");
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "focusing terminated by exception: " << x.what()
			<< std::endl;
	} catch (...) {
		std::cerr << "focusing terminated by unknown exception"
			<< std::endl;
	}
	return EXIT_FAILURE;
}
