/*
 * GPCalibrationProcess.cpp -- Process that does the calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroGuiding.h>
#include <AstroIO.h>
#include "GPCalibrationProcess.h"
#include <AstroCallback.h>
#include <AstroUtils.h>

using namespace astro::image;
using namespace astro::camera;
using namespace astro::thread;

namespace astro {
namespace guiding {

/**
 * \brief Analyze a single grid point
 *
 * Moves (relatively) to a grid point, takes an image and returns the
 * the offset as measured by the tracker.
 */
Point	GPCalibrationProcess::starAt(double ra, double dec) {
	// move the telescope to the point
	moveto(grid * ra, grid * dec);

	// take an image at that position
	ImagePtr	image;
	if (hasGuider()) {
		image = guider()->getImage();
	} else {
		imager().startExposure(exposure());
		usleep(1000000 * exposure().exposuretime());
		image = imager().getImage();
	}

	// analze the image
	Point	star = (*tracker())(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker found star at %s",
		star.toString().c_str());
	return star;
}

/**
 * \brief Send a calibration point to the callback
 */
void	GPCalibrationProcess::callback(const CalibrationPoint& calpoint) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point callback");
	if (!hasGuider()) {
		return;
	}
	guider()->callback(calpoint);
}

/**
 * \brief Send progress info to the callback
 */
void	GPCalibrationProcess::callback(const ProgressInfo& progressinfo) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(progressinfo);
}

/**
 * \brief Send the completed calibration data to the callback
 */
void	GPCalibrationProcess::callback(const CalibrationPtr calibration) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(calibration);
}

/**
 * \brief Send the image to the callback
 */
void	GPCalibrationProcess::callback(const ImagePtr& image) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(image);
}

/**
 * \brief Send an exception to the callback
 */
void	GPCalibrationProcess::callback(const std::exception& ex) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(ex);
}

/**
 * \brief Measure a given grid point
 *
 * Moves to a grid point, measures the offset seen by the tracker, then
 * returns to the original point and measures that again.
 */
void	GPCalibrationProcess::measure(int ra, int dec) {
	// skip the point ra=0, dec=0
	if ((0 == ra) && (0 == dec)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping origin");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processing grid point %d/%d", ra, dec);

	// move the telescope to the grid point corresponding to ra/dec
	Point	star = starAt(ra, dec);
	double	t = Timer::gettime() - starttime;
	CalibrationPoint	calibrationpoint(t,
					Point(grid * ra, grid * dec), star);

	// add the calibration point to the calibrator
	calibration()->add(calibrationpoint);
	addCalibrationPoint(calibrationpoint);

	// give the point to the callback
	callback(calibrationpoint);

	// move the telescope back
	star = starAt(-ra, -dec);
	t = Timer::gettime() - starttime;
	CalibrationPoint	zeropoint(t, Point(0, 0), star);

	// also add the new zero point to the calibrator
	calibration()->add(zeropoint);
	addCalibrationPoint(zeropoint);

	// give this point to the callback
	callback(zeropoint);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "measure %.0f/%.0f complete", ra, dec);
}

/**
 * \brief Compute current progress
 *
 * This esimates the progress based on the number of points already scanned
 */
double	GPCalibrationProcess::currentprogress(int ra, int dec) const {
	int	l = 2 * range + 1;
	double	maxpoints = l * l;
	return (l * (ra + range) + (dec + range) + 1) / maxpoints;
}

/**
 * \brief exception thrown when the calibration is interrupted
 */
class calibration_interrupted : public std::exception {
public:
	calibration_interrupted() { }
};

/**
 * \brief Main function of the GPCalibrationProcess class
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
 */
void	GPCalibrationProcess::main(astro::thread::Thread<GPCalibrationProcess>& _thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the main function");
	try {
		main2(_thread);
		return;
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "calibration thread terminated "
			"by %s: %s", demangle(typeid(x).name()).c_str(),
			x.what());
		return;
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "calibration thread terminated "
			"by unknown exception");
	}
	// if we get here, then the calibration process has failed, and
	// we should go back to the idle state
	// XXX
}

/**
 * \brief private part of the main method
 */
void	GPCalibrationProcess::main2(astro::thread::Thread<GPCalibrationProcess>& _thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"start calibrating: terminate = %s, guider = %s",
		_thread.terminate() ? "YES" : "NO", hasGuider() ? "YES" : "NO");
	// set the start time
	starttime = Timer::gettime();

	// send progress update for value 0
	ProgressInfo	pi;
	pi.t = 0;
	pi.progress = 0;
	pi.aborted = false;
	callback(pi);

	// grid range we want to scan
	range = 1;

	// the grid constant normally depends on the focallength and the
	// pixels size. Smaller pixels are larger focallength allow to
	// use a smaller grid constant. The default value of 10 is a good
	// choice for a 100mm guide scope and 7u pixels as for the SBIG
	// ST-i guider kit
	grid = gridconstant(_focallength, guider()->pixelsize());

	// measure the initial point
	CalibrationPoint	initialpoint(0, Point(0, 0), starAt(0, 0));
	calibration()->add(initialpoint);
	addCalibrationPoint(initialpoint);
	callback(initialpoint);

	// perform a grid search
	try {
		for (int ra = -range; ra <= range; ra++) {
			for (int dec = -range; dec <= range; dec++) {
				measure(ra, dec);
				if (_thread.terminate()) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"terminate signal received");
					throw calibration_interrupted();
				}
				// update progress indicators
				pi.t = Timer::gettime() - starttime;
				pi.progress = currentprogress(ra, dec);
				callback(pi);
			}
		}
	} catch (calibration_interrupted&) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration interrupted");
		pi.t = Timer::gettime() - starttime;
		pi.progress = 1;
		pi.aborted = true;
		callback(pi);
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration measurements complete");
	
	// now compute the calibration data, and fix the time constant
	calibration()->calibrate();
	//cal.rescale(1. / grid);

	// send the progress indicator to the end
	pi.t = Timer::gettime() - starttime;
	pi.progress = 1.0;
	callback(pi);

	// inform the callback that calibration is complete, this also
	// ensures that the guider saves the calibration
	callback(calibration());

	// the guider is now calibrated
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		calibration()->toString().c_str());
	calibrated = true;

	// signal other threads that we are done
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration complete");
}

/**
 * \brief compute the grid constant
 *
 * the grid constant normally depends on the focallength and the
 * pixels size. Smaller pixels are larger focallength allow to
 * use a smaller grid constant.
 *
 * The grid constant is chosen so as to generate an offset of about 5
 * pixels on the guide CCD. It is assumed that standard guider rate
 * of 15"/second are used.
 */
double	GPCalibrationProcess::gridconstant(double focallength,
	double pixelsize) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"grid constant for focallength = %.0fmm, pixelsize = %.1fum",
		1000 * focallength, 1000000 * pixelsize);
	double	gridconstant = 10;
	if ((focallength > 0) && (pixelsize > 0)) {
		//                           radians/pixel
		double	angular_resolution = pixelsize / focallength;
		//     radians/sec     15" / sec    * radians/degree
		double	guide_rate = ((15. / 3600.) * (M_PI / 180.));
		//      pixel/sec
		double 	pixel_rate = guide_rate / angular_resolution;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel rate: %f", pixel_rate);

		// never make the grid constant smaller than 2 (2 second
		// drives), and we ant at least 5 pixels
		gridconstant = std::max(2., ceil(5 / pixel_rate));
		gridconstant = std::min(gridconstant, 10.);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using grid time constant %.3fs",
			gridconstant);
	}
	return gridconstant;
}

/**
 * \brief Construct a guider from guider, guideport, tracker and a database
 */
GPCalibrationProcess::GPCalibrationProcess(GuiderBase *_guider,
	camera::GuidePortPtr guideport, TrackerPtr _tracker,
	persistence::Database _database)
	: GuidePortProcess(_guider, guideport, _tracker, _database) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a new calibration process");
	_focallength = 0.600;
	calibrated = false;

	// prepare a BasicCalibrator class that does the actual computation
	ControlDeviceNamePtr	gpname = guider()->guidePortDeviceName();
	calibration(CalibrationPtr(new GuiderCalibration(*gpname)));

	// create the thread
	thread(ThreadPtr(new astro::thread::Thread<GPCalibrationProcess>(this)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread constructed");
}

/**
 * \brief Destroy a calibration process
 */
GPCalibrationProcess::~GPCalibrationProcess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy calibration process");
	try {
		stop();
		wait(60);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"GPCalibrationProcess destructor throws exception: %s",
			x.what());
	}
}

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
 */
void	GPCalibrationProcess::start() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the calibration thread");
	GuidePortProcess::start();
}

/**
 * \brief Move to a grid position
 *
 * Given grid position (ra,dec), move the telescope to this grid position,
 * by actuating right ascension and declination guider ports for the 
 * corresponding number of seconds.
 */
void	GPCalibrationProcess::moveto(double ra, double dec) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "RA: raplus = %f, raminus = %f, t = %f",
		raplus, raminus, t);
	guideport()->activate(raplus, raminus, 0, 0);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC: decplus = %f, decminus = %f, t = %f",
		decplus, decminus, t);
	guideport()->activate(0, 0, decplus, decminus);
	Timer::sleep(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moveto complete");
}

} // namespace guiding
} // namespace astro
