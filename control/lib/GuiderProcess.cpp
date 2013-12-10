/*
 * GuilderProcess.cpp -- GuiderProcess implementation 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderProcess.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroUtils.h>

using namespace astro::camera;
using namespace astro::image::transform;
using namespace astro::image;

namespace astro {
namespace guiding {

/**
 * \brief create a GuiderProcess instance
 *
 * This also initializes the values for guider port activation to values that
 * compensate the drift to first order.
 */
GuiderProcess::GuiderProcess(Guider& _guider, double interval)
	: guider(_guider), _interval(interval) {
	// set a default gain
	gain = 1;

	// compute the ra/dec duty cycle to compensate the drift
	// (the vx, vy speed found in the calibration). We determine these
	// using the 
	const GuiderCalibration&	calibration = guider.calibration();

	// the default correction only neutralizes the drift
	Point	correction = calibration.defaultcorrection();
	tx = -correction.x();
	ty = -correction.y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tx = %.3fs, ty = %.3fs", tx, ty);
	if ((fabs(tx) > 1) || (fabs(ty) > 1)) {
		std::string	msg = stringprintf("default activation times "
			"%.3f, %.3f out of range", tx, ty);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// initialize thread stuff
	int	err = pthread_mutex_init(&mutex, NULL);
	if (0 != err) {
		std::string	msg = stringprintf("cannot create mutex: %s",
			strerror(err));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief Destroy the GuiderProcess
 *
 * This implicitely stops both threads, waits until they terminate, and cleans
 * up the resources allocated by them.
 */
GuiderProcess::~GuiderProcess() {
	// stop the thread specific data
	stop();

	// destroy 
}

/**
 * \brief Main guiderprocess method
 */
void	*GuiderProcess::guide_main() {
	int	rc = 0;
	do {
		// read the currently valid corrections from tx and ty,
		// this must be done while the mutex is held, or the
		// data we read my be inconsistent.
		pthread_mutex_lock(&mutex);
		double	raplus = 0, raminus = 0;
		if (tx > 0) {
			raplus = tx;
		} else {
			raminus = -tx;
		}
		double	decplus = 0, decminus = 0;
		if (ty > 0) {
			decplus = ty;
		} else {
			decminus = -ty;
		}
		pthread_mutex_unlock(&mutex);

		// now activate the guider port outputs for the times we found
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"GUIDE: activate(%.3f, %.3f, %.3f, %.3f)",
			raplus, raminus, decplus, decminus);
		guider.guiderport()->activate(raplus, raminus,
			decplus, decminus);

		// wait for one second. We do this using a wait on a condition
		// variable. The variable can also be used to signal that we
		// should terminate. First we have to compute the time when
		// the wait should end
		struct timeval	tv;
		gettimeofday(&tv, NULL);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GUIDE: now: %d.%06u",
			tv.tv_sec, tv.tv_usec);

		struct timespec	ts;
		ts.tv_sec = tv.tv_sec + 1;
		ts.tv_nsec = 1000 * tv.tv_usec;
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"GUIDE: next activity at %d.%09u",
			ts.tv_sec, ts.tv_nsec);

		// now wait
		rc = pthread_cond_timedwait(&guide.cond, &guide.mutex, &ts);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GUIDE: wait complete (%s)",
			strerror(rc));
	} while (rc == ETIMEDOUT);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GUIDE: Termination signal received");
	pthread_exit(NULL);
}

/**
 * \brief Trampolin function to launch the main method of the guider process
 */
static void	*guiderprocess_main(void *private_data) {
	GuiderProcess	*guiderprocess = (GuiderProcess *)private_data;
	return guiderprocess->guide_main();
}

/**
 * \brief Main tracker method
 */
void	*GuiderProcess::track_main() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: tracker main function started");
	while (tracking) {
		// we measure the time it takes to get an exposure. This
		// may be larger than the interval, so we need the time
		// to protect from overcorrecting
		double	starttime = Timer::gettime();

		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: start new exposure");
		// initiate an exposure
		guider.startExposure();

		// until the image is exposed
		usleep(1000000 * guider.exposure().exposuretime);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: exposure complete");

		// now retreive the image
		ImagePtr	image = guider.getImage();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: new image received");
		double	endtime = Timer::gettime();

		// use the tracker to find the tracking offset
		Point	offset = tracker->operator()(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"TRACK: current tracker offset: %s",
			offset.toString().c_str());

		// now we have to compute the correction factor for the time
		// time between images
		double	elapsed = endtime - starttime;
		double	correctiontime = elapsed;
		if (correctiontime < interval()) {
			correctiontime = interval();
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "using correction interval %f", 
			correctiontime);

		// compute the correction to tx and ty
		Point	correction = guider.calibration()(offset,
			correctiontime);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "TRACK: correction: %s",
			correction.toString().c_str());

		// compute the correction, but this must be done with tx, ty
		// protected from concurrent access, because it may be in an
		// illegal state temporarily. We also have to divide by the
		// interval, assuming that we will correct the offset by the
		// time we get the next image
		pthread_mutex_lock(&mutex);
		tx = -gain * correction.x() / correctiontime;
		if (tx > 1) { tx = 1; }
		if (tx < -1) { tx = -1; }
		ty = -gain * correction.y() / correctiontime;
		if (ty > 1) { ty = 1; }
		if (ty < -1) { ty = -1; }
		pthread_mutex_unlock(&mutex);

		// now ensure that we don't correct more often than specified
		// by the interval
		if (elapsed < interval()) {
			unsigned int	useconds
				= (interval() - elapsed) * 1000000;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"sleep %udusec for %f sec cycles",
				useconds, interval());
			usleep(useconds);
		}
	}
	pthread_exit(NULL);
}

/**
 * \brief Trampolin function to launch the main method of the guider process
 */
static void	*trackerprocess_main(void *private_data) {
	GuiderProcess	*guiderprocess = (GuiderProcess *)private_data;
	return guiderprocess->track_main();
}

/**
 * \brief Start the tracker process
 *
 * \param _tracker	the tracker to use to determine the offset
 */
bool	GuiderProcess::start(TrackerPtr _tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launching guiding threads");
	// remember the tracker
	tracker = _tracker;
	if (interval() < 1) {
		std::string	msg = stringprintf("cannot guide in %.3f "
			"second intervals: minimum 1", interval());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// initialize the tx, ty variables
	tx = 0;
	ty = 0;

	// prepare parameters for the new thread
	pthread_cond_init(&guide.cond, NULL);
	pthread_mutex_init(&guide.mutex, NULL);

	// create the mutex that protects the speed data
	pthread_mutex_init(&mutex, NULL);

	// create the tracker process
	pthread_attr_init(&guide.attr);
	pthread_create(&guide.thread, &guide.attr, trackerprocess_main, this);

	// create the tracker thrad
	tracking = true;
	pthread_attr_init(&trackattr);
	pthread_create(&track, &trackattr, guiderprocess_main, this);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding threads launched");
	return true;
}

/**
 * \brief Stop the tracker process
 */
bool	GuiderProcess::stop() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping guiding threads");
	// stop the tracker process
	tracking = false;

	// stop the guider process
	pthread_cond_signal(&guide.cond);

	// wait for the threads to terminate
	void	*result;
	pthread_join(guide.thread, &result);
	pthread_join(track, &result);

	// clean up resources used by the threads
	pthread_mutex_destroy(&guide.mutex);
	pthread_cond_destroy(&guide.cond);

	// destroy the mutex that protects the tx,ty variables
	pthread_mutex_destroy(&mutex);

	// clean up all resources needed by the thread
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding threads stopped");
	return true;
}

double	GuiderProcess::getGain() const {
	return gain;
}

void	GuiderProcess::setGain(double _gain) {
	gain = _gain;
}

} // namespace guiding
} // namespace astro

