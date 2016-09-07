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
AOCalibrationProcess::AOCalibrationProcess(GuiderBase *guider,
	camera::AdaptiveOpticsPtr adaptiveoptics,
	TrackerPtr tracker, persistence::Database database)
	: CalibrationProcess(guider, tracker, database),
	  _adaptiveoptics(adaptiveoptics) {
	// prepare the calibration 
	ControlDeviceNamePtr	cdname = guider->adaptiveOpticsDeviceName();
	calibration(CalibrationPtr(new AdaptiveOpticsCalibration(*cdname)));

	// start the thread
	thread(thread::ThreadPtr(new thread::Thread<AOCalibrationProcess>(this)));
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

	// set up the timer
	double	starttime = Timer::gettime();

	// Progress indicator data
	ProgressInfo	pi;
	pi.aborted = false;
	pi.progress = 0;
	pi.t = Timer::gettime() - starttime;
	callback(pi);

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
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new point %d: %s", i,
				offset.toString().c_str());
			
			// position the adaptive optics unit
			_adaptiveoptics->set(offset);
			usleep(100000); // allow the AO unit to settle

			// test whether the termination has been requested
			if (thread.terminate()) {
				throw aocalibration_interrupted();
			}

			// take an image
			ImagePtr	image = guider()->getImage();
			Point	star = (*tracker())(image);

			// add the calibration point
			double	t = Timer::gettime() - starttime;
			CalibrationPoint	calpoint(t, offset, star);
			calibration()->add(calpoint);
			callback(calpoint);

			// update the progress indicator
			pi.t = Timer::gettime() - starttime;
			pi.progress = (i + 1.0) / npoints;
			callback(pi);
		}
	} catch (const aocalibration_interrupted& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "termination requested");
		_adaptiveoptics->set(Point());
		usleep(100000); // allow the AO unit to settle
		pi.t = Timer::gettime() - starttime;
		pi.progress = 1;
		pi.aborted = true;
		callback(pi);
		return;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exception caused abort: %s",
			x.what());
		pi.t = Timer::gettime() - starttime;
		pi.aborted = true;
		callback(pi);
		return;
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown exception cased abort");
		pi.t = Timer::gettime() - starttime;
		pi.aborted = true;
		callback(pi);
		return;
	}

	// we have found a calibration
	calibration()->calibrate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration: %s",
		calibration()->toString().c_str());
	callback(calibration());
}

/**
 * \brief Send a calibration point to the callback
 */
void	AOCalibrationProcess::callback(const CalibrationPoint& calpoint) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration point callback");
	if (!hasGuider()) {
		return;
	}
	guider()->callback(calpoint);
}

/**
 * \brief Send progress info to the callback
 */
void	AOCalibrationProcess::callback(const ProgressInfo& progressinfo) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(progressinfo);
}

/**
 * \brief Send the completed calibration data to the callback
 */
void	AOCalibrationProcess::callback(const CalibrationPtr calibration) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(calibration);
}

/**
 * \brief Send the image to the callback
 */
void	AOCalibrationProcess::callback(const ImagePtr& image) {
	if (!hasGuider()) {
		return;
	}
	guider()->callback(image);
}

} // namespace guiding
} // namespace astro

