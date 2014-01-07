/*
 * CalibrationProcess.cpp -- Process that does the calibration
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroGuiding.h>
#include <AstroIO.h>
#include <CalibrationProcess.h>
#include <AstroCallback.h>
#include <AstroUtils.h>

using namespace astro::image;
using namespace astro::camera;

namespace astro {
namespace guiding {

/**
 * \brief Analize a single grid point
 *
 * Moves (relatively) to a grid point, takes an image and returns the
 * the offset as measured by the tracker.
 */
Point	CalibrationProcess::pointat(double ra, double dec) {
	// move the telescope to the point
	moveto(grid * ra, grid * dec);

	// take an image at that position
	imager().startExposure(exposure());
	usleep(1000000 * exposure().exposuretime);
	ImagePtr	image = guider().getImage();

	// analze the image
	Point	point = (*tracker())(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker found %s", point.toString().c_str());
	return point;
}

/**
 * \brief Measure a given grid point
 *
 * Moves to a grid point, measures the offset seen by the tracker, then
 * returns to the original point and measures that again.
 */
void	CalibrationProcess::measure(GuiderCalibrator& calibrator,
		double ra, double dec) {
	// move the telescope to the grid point corresponding to ra/dec
	Point	point = pointat(ra, dec);
	double	t = Timer::gettime();
	calibrator.add(t, Point(ra, dec), point);

	// move the telescope back
	point = pointat(-ra, -dec);
	t = Timer::gettime();
	calibrator.add(t, Point(0, 0), point);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "measure %.0f/%.0f complete", ra, dec);
}

/**
 * \brief Compute current progress
 *
 * This esimates the progress based on the number of points already scanned
 */
double	CalibrationProcess::currentprogress(int ra, int dec) const {
	double	maxpoints = (2 * range + 1) * (2 + range + 1);
	return ((2 * range + 1) * (ra + range) + (dec + range)) / maxpoints;
}

/**
 * \brief exception thrown when the calibration is interrupted
 */
class calibration_interrupted : public std::exception {
public:
	calibration_interrupted() { }
};

/**
 * \brief Main function of the CalibrationProcess class
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
void	CalibrationProcess::main(GuidingThread<CalibrationProcess>& _thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating: terminate = %s",
		_thread.terminate() ? "YES" : "NO");

	// grid range we want to scan
	range = 1;

	// the grid constant normally depends on the focallength and the
	// pixels size. Smaller pixels are larger focallength allow to
	// use a smaller grid constant. The default value of 10 is a good
	// choice for a 100mm guide scope and 7u pixels as for the SBIG
	// ST-i guider kit
	grid = gridconstant(_focallength, _pixelsize);

	// prepare a GuiderCalibrator class that does the actual computation
	GuiderCalibrator	calibrator;

	// perform a grid search
	try {
		for (int ra = -range; ra <= range; ra++) {
			for (int dec = -range; dec <= range; dec++) {
				measure(calibrator, ra, dec);
				if (_thread.terminate()) {
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"terminate signal received");
					throw calibration_interrupted();
				}
				_progress = currentprogress(ra, dec);
			}
		}
	} catch (calibration_interrupted&) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration interrupted");
		return;
	}
	
	// now compute the calibration data, and fix the time constant
	GuiderCalibration	cal = calibrator.calibrate();
	cal.rescale(1. / grid);
	guider().calibration(cal);

	// the guider is now calibrated
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		guider().calibration().toString().c_str());
	calibrated = true;

	// signal other threads that we are done
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration complete");
	_progress = 1.0;
}

/**
 * \brief compute the grid constant
 *
 * the grid constant normally depends on the focallength and the
 * pixels size. Smaller pixels are larger focallength allow to
 * use a smaller grid constant. The default value of 10 seems to be
 * a good  choice for a 100mm guide scope and 7u pixels as for the
 * SBIG ST-i guider kit
 */
double	CalibrationProcess::gridconstant(double focallength,
	double pixelsize) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"grid constant for focallength = %.0fmm, pixelsize = %.1fum",
		1000 * focallength, 1000000 * pixelsize);
	double	gridconstant = 10;
	if ((focallength > 0) && (pixelsize > 0)) {
		// the angular_default is the angular resolution (in radians)
		// that is suitable for 10 second drives to calibrate. If 
		// the pixels are smaller or the focal length is larger,
		// then a shorter time is ok
		double	angular_default = 0.0000074 / 0.100;
		double	angular_resolution = pixelsize / focallength;

		// never make the grid constant smaller than 2 (2 second
		// drives)
		gridconstant = std::max(2.,
			10. * angular_resolution / angular_default);
		gridconstant = std::min(gridconstant, 10.);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using grid constant %.3f",
			gridconstant);
	}
	return gridconstant;
}

/**
 * \brief Construct a guider from 
 */
CalibrationProcess::CalibrationProcess(Guider& _guider, TrackerPtr _tracker)
	: GuidingProcess(_guider, _tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct a new calibration process");
	_focallength = 0.600;
	_pixelsize = 0.000010;
	calibrated = false;
	_progress = 0;
	// create the thread
	thread(ThreadPtr(new GuidingThread<CalibrationProcess>(*this)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread constructed");
}

/**
 * \brief Destroy a calibration process
 */
CalibrationProcess::~CalibrationProcess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy calibration process");
	try {
		stop();
		wait(60);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"CalibrationProcess destructor throws exception: %s",
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
 *
 * This method may require additional parameters to be completely useful.
 * \param focallength    focallength of guide scope in mm
 * \param pixelsize      size of pixels in um
 */
void	CalibrationProcess::calibrate(double focallength, double pixelsize) {
	// remember the grid constants
	_focallength = focallength;
	_pixelsize = pixelsize;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the calibration thread");
	start();
}

/**
 * \brief Move to a grid position
 *
 * Given grid position (ra,dec), move the telescope to this grid position,
 * by actuating right ascension and declination guider ports for the 
 * corresponding number of seconds.
 */
void	CalibrationProcess::moveto(double ra, double dec) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DEC: decplus = %f, decminus = %f, t = %f",
		decplus, decminus, t);
	guiderport()->activate(0, 0, decplus, decminus);
	Timer::sleep(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moveto complete");
}

} // namespace guiding
} // namespace astro
