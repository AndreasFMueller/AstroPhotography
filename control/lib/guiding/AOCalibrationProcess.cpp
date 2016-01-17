/*
 * AOCalibrationProcess.cpp -- adaptive optics calibration implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AOCalibrationProcess.h>

namespace astro {
namespace guiding {

/**
 * \brief Create a calibration process
 *
 * This constructor is based on a guider class, which also means that 
 * the calibration can be written back to the guider
 */
AOCalibrationProcess::AOCalibrationProcess(Guider *guider, TrackerPtr tracker,
	persistence::Database database)
	: BasicProcess(guider, tracker, database) {
}

/**
 * \brief Create a clibration process
 *
 * Create a calibration process that does not depend on a guider class
 * instance.
 */
AOCalibrationProcess::AOCalibrationProcess(const camera::Exposure& exposure, 
	camera::Imager& imager, TrackerPtr tracker,
	camera::AdaptiveOpticsPtr adaptiveoptics,
	persistence::Database database)
	: BasicProcess(exposure, imager, tracker, database),
	  _adaptiveoptics(adaptiveoptics) {
}

/**
 * \brief auxiliary exception class to signal that the thread was interrupted
 */
class aocalibration_interrupted {
public:
	aocalibration_interrupted() { }
};

/**
 * \brief main function 
 */
void	AOCalibrationProcess::main(thread::Thread<AOCalibrationProcess>& thread) {
	// start the thread
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start AO calibration for %s",
		_adaptiveoptics->name().toString().c_str());

	// prepare the calibrator and the timer
	BasicCalibrator	calibrator;
	double	starttime = Timer::gettime();

	// go to a number of points and measure the offset
	try {
		const int	npoints = 17;
		const int	delta = 12;
		const double	dangle = 2 * M_PI / npoints;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "measuring %d points", npoints);
		for (int i = 0; i < npoints; i++) {
			// test whether the termination has been requested
			if (thread.terminate()) {
				throw aocalibration_interrupted();
			}

			// compute the point
			double	angle = ((i * delta) % npoints) * dangle;
			Point	offset = Point(angle) * 0.5;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "point %d: %s",
				offset.toString().c_str());
			
			// position the adaptive optics unit
			_adaptiveoptics->set(offset);
			usleep(100000); // allow the AO unit to settle

			// test whether the termination has been requested
			if (thread.terminate()) {
				throw aocalibration_interrupted();
			}

			// take an image
			ImagePtr	image = imager().getImage();
			Point	star = (*tracker())(image);

			// add the calibration point
			double	t = Timer::gettime() - starttime;
			CalibrationPoint	calpoint(t, offset, star);
			calibrator.add(calpoint);
		}
	} catch (const aocalibration_interrupted& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "termination requested");
		_adaptiveoptics->set(Point());
		usleep(100000); // allow the AO unit to settle
		return;
	}

	// we have found a calibration
	BasicCalibration	cal = calibrator.calibrate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		cal.toString().c_str());
}

} // namespace guiding
} // namespace astro
