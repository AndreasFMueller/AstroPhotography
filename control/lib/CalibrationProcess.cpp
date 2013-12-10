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
using namespace astro::callback;

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
	ImagePtr	image = guider().getImage();

	// analze the image
	Point	point = (*tracker())(image);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibrating");

	// grid range we want to scan
	int range = 1;

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
					throw calibration_interrupted();
				}
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
}

/**
 * \brief compute the grid constant
 *
 * the grid constant normally depends on the focallength and the
 * pixels size. Smaller pixels are larger focallength allow to
 * use a smaller grid constant. The default value of 10 is a good
 * choice for a 100mm guide scope and 7u pixels as for the SBIG
 * ST-i guider kit
 */
double	CalibrationProcess::gridconstant(double focallength,
	double pixelsize) const {
	double	gridconstant = 10;
	if ((focallength > 0) && (pixelsize > 0)) {
		gridconstant = 10 * (pixelsize / 7.4) / (focallength / 100);
		if (gridconstant < 2) {
			gridconstant = 2;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using grid constant %f",
			gridconstant);
	}
	return gridconstant;
}

/**
 * \brief Construct a guider from 
 */
CalibrationProcess::CalibrationProcess(Guider& _guider, TrackerPtr _tracker)
	: GuidingProcess(_guider, _tracker) {
	calibrated = false;
	// default exposure settings
	exposure().exposuretime = 1.;
	exposure().frame = guider().ccd()->getInfo().getFrame();
	calibrated = false;
	// create the thread
	thread = ThreadPtr(new GuidingThread<CalibrationProcess>(*this));
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

/**
 * \brief start the calibration process
 */
void	CalibrationProcess::start() {
	thread->start();
}

/**
 * \brief stop the process
 */
void	CalibrationProcess::stop() {
	thread->stop();
}

/**
 * \brief wait for the thread to terminate
 */
bool	CalibrationProcess::wait(double timeout) {
	return thread->wait(timeout);
}


} // namespace guiding
} // namespace astro
