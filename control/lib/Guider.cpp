/*
 * Guiding.cpp -- classes implementing guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroIO.h>
#include <GuiderProcess.h>
#include <CalibrationProcess.h>
#include <AstroCallback.h>
#include <AstroUtils.h>

using namespace astro::image;
using namespace astro::camera;
using namespace astro::callback;

namespace astro {
namespace guiding {

//////////////////////////////////////////////////////////////////////
// Callback for images retrieved (to help analysis of guider problems)
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Guider implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a guider from 
 */
Guider::Guider(CameraPtr camera, CcdPtr ccd, GuiderPortPtr guiderport)
	: _camera(camera), _guiderport(guiderport), _imager(ccd) {
	// default exposure settings
	exposure().exposuretime = 1.;
	exposure().frame = ccd->getInfo().getFrame();
	//gridconstant = 10;
}

/**
 * \brief Retrieve the state 
 */
GuiderState	Guider::state() const {
	GuiderState	result = _state;
	return result;
}

#if 0
/**
 * \brief Calibrate the guiding system
 *
 * This method assumes that the observed star position depends linearly
 * on time and the applied correction. It then performs several position
 * measurements and solves for the equation. The resulting matrix should have
 * two nearly perpendicular columns.
 *
 * The mesurements are placed in a grid pattern with coordinate (ra, dec)
 * corresponding to a point that can be reached from the initial position
 * by speeing up (down for negative values) the right ascension/declination
 * motors for ra resp. dec seconds. After each measurement, we return to the
 * central position.
 *
 * This method may require additional parameters to be completely useful.
 * \param focallength    focallength of guide scope in mm
 * \param pixelsize      size of pixels in um
 */
bool	Guider::calibrate(TrackerPtr tracker,
		double focallength, double pixelsize) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating");

	// grid range we want to scan
	int range = 1;

	// the grid constant normally depends on the focallength and the
	// pixels size. Smaller pixels are larger focallength allow to
	// use a smaller grid constant. The default value of 10 is a good
	// choice for a 100mm guide scope and 7u pixels as for the SBIG
	// ST-i guider kit
	if ((focallength > 0) && (pixelsize > 0)) {
		gridconstant = 10 * (pixelsize / 7.4) / (focallength / 100);
		if (gridconstant < 2) {
			gridconstant = 2;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using grid constant %f",
			gridconstant);
	}

	// prepare a GuiderCalibrator class that does the actual computation
	GuiderCalibrator	calibrator;

	// perform a grid search
	for (int ra = -range; ra <= range; ra++) {
		for (int dec = -range; dec <= range; dec++) {
			// move the telescope to the grid position
			moveto(gridconstant * ra, gridconstant * dec);
			imager().startExposure(exposure());
			ImagePtr	image = getImage();
			Point	point = (*tracker)(image);
			double	t = Timer::gettime();
			calibrator.add(t, Point(ra, dec), point);
			// move the telescope back
			moveto(-gridconstant * ra, -gridconstant * dec);
			imager().startExposure(exposure());
			image = getImage();
			point = (*tracker)(image);
			t = Timer::gettime();
			calibrator.add(t, Point(0, 0), point);
		}
	}
	
	// now compute the calibration data
	calibration(calibrator.calibrate());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		calibration().toString().c_str());
	_calibrated = true;

	// fix time constant
	calibration().a[0] /= gridconstant;
	calibration().a[1] /= gridconstant;
	calibration().a[3] /= gridconstant;
	calibration().a[4] /= gridconstant;

	// are we now calibrated?
	return _calibrated;
}

/**
 * \brief Move to a grid position
 *
 * Given grid position (ra,dec), move the telescope to this grid position,
 * by actuating right ascension and declination guider ports for the 
 * corresponding number of seconds.
 */
void	Guider::moveto(double ra, double dec) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moveto (%f, %f)", ra, dec);
	double	t = 0;
	double	raplus = 0;
	double	raminus = 0;
	double	decplus = 0;
	double	decminus = 0;

	if (ra > 0) {
		raplus = ra;
	} else {
		raminus = -ra;
	}
	if (raplus > t) {
		t = raplus;
	}
	if (raminus > t) {
		t = raminus;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA: raplus = %f, raminus = %f",
		raplus, raminus);
	guiderport()->activate(raplus, raminus, 0, 0);
	Timer::sleep(t);

	t = 0;
	if (dec > 0) {
		decplus = dec;
	} else {
		decminus = -dec;
	}
	if (decminus > t) {
		t = decminus;
	}
	if (decplus > t) {
		t = decplus;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC: decplus = %f, decminus = %f",
		decplus, decminus);
	guiderport()->activate(0, 0, decplus, decminus);
	Timer::sleep(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moveto complete");
}
#endif

/**
 * \brief Set a calibration
 */
void	Guider::calibration(const GuiderCalibration& calibration) {
	_state.addCalibration();
	_calibration = calibration;
}

/**
 * \brief start an asynchronous calibration process
 *
 * This method first checks that no other calibration thread is running,
 * and if so, starts a new thread.
 */
void	Guider::startCalibration(TrackerPtr tracker, double focallength,
		double pixelsize) {
	// go into the calibrating state
	_state.startCalibrating();
	
	// first check whether there already is a calibration process
	// running
	if (guiderprocess) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"calibration already in progress: %f",
			calibrationProgress());
		throw std::runtime_error("calibration already in progress");
	}

	// now create a new calibration process
	calibrationprocess = CalibrationProcessPtr(
		new CalibrationProcess(*this, tracker));

	// start the calibration. This will launch the separate 
	calibrationprocess->calibrate(focallength, pixelsize);
}

/**
 * \brief inquire about the current state of the calibration process
 */
double	Guider::calibrationProgress() {
	if (_state != calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	return calibrationprocess->progress();
}

/**
 * \brief cancel a calibration that is still in progress
 */
void	Guider::cancelCalibration() {
	if (_state != calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	calibrationprocess->stop();
}

/**
 * \brief wait for the calibration to complete
 */
bool	Guider::waitCalibration(double timeout) {
	if (_state != calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	return calibrationprocess->wait(timeout);
}


/**
 * \brief start an exposure
 */
void	Guider::startExposure() {
	imager().startExposure(exposure());
}

/**
 * \brief get the image
 */
ImagePtr	Guider::getImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage() called");
	ImagePtr	image = imager().getImage();
	callbackImage(image);
	return image;
}

/**
 * \brief send the image to the callback
 */
void	Guider::callbackImage(ImagePtr image) {
	if (newimagecallback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending new image to callback");
		GuiderNewImageCallbackData	*argp = 
			new GuiderNewImageCallbackData(image);
		CallbackDataPtr	arg(argp);
		newimagecallback->operator()(arg);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback return");
	}
}

/**
 * \brief start tracking
 */
bool	Guider::start(TrackerPtr tracker) {
	// create a GuiderProcess instance
	guiderprocess = GuiderProcessPtr(new GuiderProcess(*this));
	guiderprocess->start(tracker);
	return true;
}

bool	Guider::stop() {
	guiderprocess->stop();
	return true;
}

} // namespace guiding
} // namespace astro
