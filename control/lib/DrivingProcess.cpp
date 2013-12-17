/*
 * DrivingProcess.cpp -- thread handling the guider port during guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DrivingProcess.h>
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
DrivingProcess::DrivingProcess(Guider& _guider)
	: GuidingProcess(_guider, TrackerPtr()) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new DrivingProcess");
	pthread_mutex_init(&mutex, NULL);
	tx = 0;
	ty = 0;
	_interval = 1;
}

/**
 * \brief Set the interval
 *
 * This non-inline implementation is necessary to ensure proper locking of
 * the shared _interval variable.
 */
void	DrivingProcess::interval(const double& i) {
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
DrivingProcess::~DrivingProcess() {
	try {
		stop();
		wait(_interval + 1);
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during destructor: %s",
			x.what());
	}
	pthread_mutex_destroy(&mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DrivingProcess terminated");
}

/**
 * \brief signal the new correction to the driving thread
 *
 * The variables tx and ty are needed by the main method so we must
 * properly lock that method out when we change them.
 */
void	DrivingProcess::setCorrection(const double& _tx, const double& _ty) {
	GuidingLock	lock(&mutex);
	tx = _tx;
	ty = _ty;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating tx = %f, ty = %f", tx, ty);
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
void	DrivingProcess::main(GuidingThread<DrivingProcess>& thread) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GUIDE: thread main function starts");
	do {
		double	raplus = 0, raminus = 0;
		double	decplus = 0, decminus = 0;
		// read the currently valid corrections from tx and ty,
		// this must be done while the mutex is held, or the
		// data we read my be inconsistent.
		{
			GuidingLock	lock(&mutex);
			if (tx > 0) {
				raplus = tx * _interval;
			} else {
				raminus = -tx * _interval;
			}
			if (ty > 0) {
				decplus = ty * _interval;
			} else {
				decminus = -ty * _interval;
			}
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
