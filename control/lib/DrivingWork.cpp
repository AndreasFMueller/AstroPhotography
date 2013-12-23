/*
 * DrivingWork.cpp -- thread handling the guider port during guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DrivingWork.h>
#include <AstroUtils.h>

namespace astro {
namespace guiding {

/**
 * \brief Construct a driving process thread
 *
 * This initializes the data structurs, but does not launch a thread
 * on the main function. This will be done by the super class' start
 * method.
 */
DrivingWork::DrivingWork(Guider& _guider)
	: GuidingProcess(_guider, TrackerPtr()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new DrivingWork");
	pthread_mutex_init(&mutex, NULL);
	defaultx = 0;
	defaulty = 0;
	totalx = 0;
	totaly = 0;
	stepx = stepy = 1;
	_interval = 1;
}

/**
 * \brief Set the interval
 *
 * This non-inline implementation is necessary to ensure proper locking of
 * the shared _interval variable.
 */
void	DrivingWork::interval(const double& i) {
	GuidingLock	lock(&mutex);
	_interval = i;
}

/**
 * \brief destroy the driving thread
 *
 * We must first stop the thread and wait for it's termination before we
 * can destroy the mutex. This is why we cannot simply stop the thread
 * in the superclass destructor.
 */
DrivingWork::~DrivingWork() {
	try {
		stop();
		wait(_interval + 1);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during destructor: %s",
			x.what());
	}
	pthread_mutex_destroy(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DrivingWork terminated");
}

/**
 * \brief signal the new correction to the driving thread
 *
 * The variables tx and ty are needed by the main method so we must
 * properly lock that method out when we change them.
 */
void	DrivingWork::setCorrection(const double& _tx, const double& _ty) {
	GuidingLock	lock(&mutex);
	totalx = fabs(_tx);
	totaly = fabs(_ty);
	stepx = (_tx > 0) ? 1 : -1;
	stepy = (_ty > 0) ? 1 : -1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating totalx = %f, totaly = %f",
		totalx, totaly);
}

/**
 * \brief Set default correction
 */
void	DrivingWork::defaultCorrection(const double& _tx, const double& _ty) {
	defaultx = _tx;
	defaulty = _ty;
}

/**
 * \brief driving thread main function
 *
 * The main function works in a loop until the thread is terminated and
 * keeps feeding the guiderport with control commands based on the settings
 * of the tx and ty values. The tx and ty values are signed duty cycle numbers
 * for the guider port, 1 means that the corresponding plus signal of the
 * guiderport should be activated all the time. The loop is timed by the
 * _interval variable, so the method just computes the time to activate
 * the guiderport and then goes to sleep for the rest of the interval.
 */
void	DrivingWork::main(GuidingThread<DrivingWork>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GUIDE: thread main function starts");
	do {
		double	raplus = 0, raminus = 0;
		double	decplus = 0, decminus = 0;
		// read the currently valid corrections from tx and ty,
		// this must be done while the mutex is held, or the
		// data we read my be inconsistent.
		{
			GuidingLock	lock(&mutex);
			if (totalx > 0) {
				double	dx = std::min(_interval, totalx);
				if (stepx > 0) {
					raplus = dx;
				} else {
					raminus = dx;
				}
				totalx -= dx;
			} else {
				if (defaultx > 0) {
					raplus = defaultx * _interval;
				} else {
					raminus = -defaultx * _interval;
				}
			}
			if (totaly > 0) {
				double	dy = std::min(_interval, totaly);
				if (stepy > 0) {
					decplus = dy;
				} else {
					decminus = dy;
				}
				totaly -= dy;
			} else {
				if (defaulty > 0) {
					decplus = defaulty * _interval;
				} else {
					decminus = -defaulty * _interval;
				}
			}
			if (totalx < 0) { totalx = 0.; }
			if (totaly < 0) { totaly = 0.; }
		}

		// now activate the guider port outputs for the times we found
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"GUIDE: activate(%.3f, %.3f, %.3f, %.3f)",
			raplus, raminus, decplus, decminus);
		guider().guiderport()->activate(raplus, raminus,
			decplus, decminus);

		// wait for one second.
		Timer::sleep(_interval);

		// checking for termination signal
	} while (!thread.terminate());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GUIDE: Termination signal received");
}

} // namespace guiding
} // namespace astro
