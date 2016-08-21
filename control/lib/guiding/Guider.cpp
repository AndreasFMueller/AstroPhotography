/*
 * Guider.cpp -- Guider class implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroIO.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <AstroAdapter.h>

#include "CalibrationProcess.h"
#include "CalibrationPersistence.h"
#include "GuiderCalibrationRedirector.h"
#include "TrackingProcess.h"

using namespace astro::image;
using namespace astro::camera;
using namespace astro::callback;
using namespace astro::persistence;

namespace astro {
namespace guiding {

/**
 * \brief Construct a guider from 
 *
 * Since the guider includes an exposure, it also initializes the exposure
 * to some default values. The default exposure time is 1 and the
 * default frame is the entire CCD area.
 */
Guider::Guider(const GuiderName& guidername,
	CcdPtr ccd, GuiderPortPtr guiderport, AdaptiveOpticsPtr adaptiveoptics,
	Database database)
	: GuiderBase(guidername, ccd, database), _guiderport(guiderport),
	  _adaptiveoptics(adaptiveoptics) {
	// default exposure settings
	exposure().exposuretime(1.);
	exposure().frame(ccd->getInfo().getFrame());

	// default focallength
	_focallength = 1;

	// We have to install a callback for calibrations
	CallbackPtr	calcallback(new GuiderCalibrationRedirector(this));
	addGuidercalibrationCallback(calcallback);
	addProgressCallback(calcallback);

	// create control devices
	if (guiderport) {
		guiderPortDevice = ControlDevicePtr(
			new ControlDevice<GuiderPort, GuiderCalibration, GP>(this,
				guiderport, database));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port control device");
	}
	if (adaptiveoptics) {
		adaptiveOpticsDevice = ControlDevicePtr(
			new ControlDevice<AdaptiveOptics,
				AdaptiveOpticsCalibration, AO>(this,
					adaptiveoptics, database));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "AO control device");
	}

	// at this point the guider is sufficiently configured, although
	// this configuration is not sufficient for guiding
	_state.configure();
}

/**
 * \brief update progress value
 */
void	Guider::calibrationProgress(double p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "PROGRESS %f", p);
	_progress = p;
}

/**
 * \brief Retrieve the state 
 *
 * Get the guider state. The guider keeps state information in the guider
 * state machine, so we have to convert that to the Guider::state constants.
 * This is done by the cast operator of the GuiderStateMachine class.
 */
Guide::state	Guider::state() {
	Guide::state	result = _state;
	// if the state is guiding or calibrating, we should check whether
	// that process is still doing it
	switch (result) {
	case Guide::calibrating:
		if ((guiderPortDevice) && (guiderPortDevice->calibrating())) {
			return result;
		}
		if ((adaptiveOpticsDevice) && (adaptiveOpticsDevice->calibrating())) {
			return result;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "apparaently the calibration "
			"process has gone away");
		_state.failCalibration();
		break;
	case Guide::guiding:
		if (trackingprocess) {
			if (trackingprocess->isrunning()) {
				return result;
			}
			trackingprocess.reset();
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "apparaently the guiding "
			"process has gone away");
		_state.stopGuiding();
		break;
	default:
		return result;
	}
	Guide::state	resultnew = _state;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "state has changed from %s to %s",
		Guide::state2string(result).c_str(),
		Guide::state2string(resultnew).c_str());
	return resultnew;
}

/**
 * \brief Cleanup for calibration processes
 *
 * If nobody waits for a calibration process, e.g. when the calibration
 * is running in a remote process, we still may want to start a new
 * calibration if the previous calibration is complete. This method
 * is intended to cleanup an old calibration process if it has already
 * terminated.
 */
void	Guider::calibrationCleanup() {
	// if we are already calibrating, we should not cleanup
	if (state() == Guide::calibrating) {
		return;
	}
}

/**
 * \brief start an asynchronous calibration process
 *
 * This method first checks that no other calibration thread is running,
 * and if so, starts a new thread.
 */
int	Guider::startCalibration(ControlDeviceType type, TrackerPtr tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration for %s",
		type2string(type).c_str());
	// make sure we have a tracker
	if (!tracker) {
		debug(LOG_ERR, DEBUG_LOG, 0, "tracker not defined");
		throw BadState("tracker not set");
	}

	// are we in the correct state
	if (!_state.canStartCalibrating()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start calibrating");
		throw BadState("wrong state");
	}
	_progress = 0;

	// start calibration
	if ((type == GP) && guiderPortDevice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuiderPort calibration");
		_state.startCalibrating();
		guiderPortDevice->setParameter("focallength", focallength());
		return guiderPortDevice->startCalibration(tracker);
	}

	if ((type == AO) && adaptiveOpticsDevice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start AO calibration");
		_state.startCalibrating();
		return adaptiveOpticsDevice->startCalibration(tracker);
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot calibrate, no device");
	throw BadState("bad state");
}

/**
 * \brief save a guider calibration
 *
 * This method is called at the end of a calibration run. Since the 
 * control device already has saved the calibration data in the database,
 * this method only needs to update the guider state.
 */
void	Guider::saveCalibration(const GuiderCalibration& cal) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving completed calibration %d",
		cal.calibrationid());
	if (!_state.canAcceptCalibration()) {
		return;
	}
	_state.addCalibration();
}

void	Guider::forgetCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "forgetting incomplete calibration");
	if (_state.canFailCalibration()) {
		_state.failCalibration();
		return;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot fail calibration");
	throw BadState("cannot forget failed calibration");
}

/**
 * \brief use a calibration from the database
 *
 * This method retrieves a calibration from the database by its id, and 
 * applies it to the appropriate control device depending on the type found
 * in the database.
 */
void	Guider::useCalibration(int calid) {
	if (!_state.canAcceptCalibration()) {
		throw BadState("cannot accept calibration now");
	}
	CalibrationStore	store(database());
	if (store.contains(calid, GP)) {
		_state.addCalibration();
		guiderPortDevice->calibrationid(calid);
		return;
	}
	if (store.contains(calid, AO)) {
		_state.addCalibration();
		adaptiveOpticsDevice->calibrationid(calid);
		return;
	}
	std::string	cause = stringprintf("calibration %d not found", calid);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
	throw NotFound(cause);
}

/**
 * \brief Uncalibrate a control device
 *
 * When guiding is started, all the calibrated control devices are used
 * for guiding. But in some cases one may no longer want to use a device,
 * e.g. an adaptive optics device. To turn such a device off, one needs
 * to uncalibrate it. We don't loose anything by uncalibrating, as we can
 * always recover the calibration from the database and calibrate again.
 * If both devices are uncalibrated after this operation, then the guider
 * goes into the state 'idle' which means that no guiding is possible.
 */
void	Guider::unCalibrate(ControlDeviceType type) {
	// make sure we are not guiding or calibrating
	if ((_state == Guide::calibrating) || (_state == Guide::guiding)) {
		std::string	cause
			= astro::stringprintf("cannot uncalibrate while %s",
				type2string(type).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw BadState(cause);
	}

	// now uncalibrate the selected device
	switch (type) {
	case GP:
		guiderPortDevice->calibrationid(-1);
		break;
	case AO:
		adaptiveOpticsDevice->calibrationid(-1);
		break;
	}

	// if neither device is no calibrated, go into the idle state
	if ((!guiderPortDevice->iscalibrated())
		&& (!adaptiveOpticsDevice->iscalibrated())) {
		_state.configure();
	}
}

/**
 * \brief cancel a calibration that is still in progress
 */
void	Guider::cancelCalibration() {
	if (_state != Guide::calibrating) {
		throw BadState("not currently calibrating");
	}
	if (guiderPortDevice) {
		if (guiderPortDevice->calibrating()) {
			guiderPortDevice->cancelCalibration();
		}
	}
	if (adaptiveOpticsDevice) {
		if (adaptiveOpticsDevice->calibrating()) {
			adaptiveOpticsDevice->cancelCalibration();
		}
	}
}

/**
 * \brief wait for the calibration to complete
 */
bool	Guider::waitCalibration(double timeout) {
	if (_state != Guide::calibrating) {
		throw BadState("not currently calibrating");
	}
	// only one device can be calibrating at a time, so we try them in turn
	if (guiderPortDevice) {
		if (guiderPortDevice->calibrating()) {
			return guiderPortDevice->waitCalibration(timeout);
		}
	}
	if (adaptiveOpticsDevice) {
		if (adaptiveOpticsDevice->calibrating()) {
			return adaptiveOpticsDevice->waitCalibration(timeout);
		}
	}
	// if no device is calibrating, we immediately return with true
	// since we checked for the state at the beginning, we shouldn't
	// ever arrive at this point
	return true;
}

/**
 * \brief get a default tracker
 *
 * This is not the only possible tracker to use with the guiding process,
 * but it works currently quite well.
 *
 * \param point		start to track, in absolute coordinates
 */
TrackerPtr	Guider::getTracker(const Point& point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get Tracker for star at %s",
		point.toString().c_str());
	// get the image origin
	astro::camera::Exposure exp = exposure();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: %s",
		exp.origin().toString().c_str());
#if 0
	astro::Point    difference = point - exp.origin();
	int	x = difference.x();
	int	y = difference.y();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "relative coordinates of star: (%d,%d)",
		x, y);
#else
	int	x = point.x();
	int	y = point.y();
#endif
	astro::image::ImagePoint        trackerstar(x, y);
	astro::image::ImageRectangle    trackerrectangle(exp.size());
	astro::guiding::TrackerPtr      tracker(
		new astro::guiding::StarTracker(trackerstar,
			trackerrectangle, 10));
	return tracker;
}

TrackerPtr	Guider::getNullTracker() {
	return TrackerPtr(new NullTracker());
}

TrackerPtr	Guider::getPhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a standard phase tracker");
	return TrackerPtr(new PhaseTracker<adapter::IdentityAdapter<double> >());
}

TrackerPtr	Guider::getDiffPhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a differential phase tracker");
	return TrackerPtr(new PhaseTracker<adapter::DerivativeNormAdapter<double> >());
}

TrackerPtr	Guider::getLaplaceTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a laplace tracker");
	return TrackerPtr(new PhaseTracker<adapter::LaplaceAdapter<double> >());
}

TrackerPtr	Guider::getLargeTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a large tracker");
	return TrackerPtr(new LargeTracker());
}

/**
 * \brief start tracking
 *
 * Guiding uses all configured devices. If the adaptive optics unit is not
 * configured, only the guider port is used. Two intervals need to be
 * provided. The gpinterval is the time between guider port actions. It is
 * assumed that the guider port reacts very slowly, so gpinterval is usually
 * about an order of magnitude larger than the aointerval, that controls the
 * update interval for the adaptive optics unit. If the aointerval is zero,
 * then the adaptive optics process updates as quickly as possible, essentially
 * limited by the exposure time and the time it takes to download an imge from
 * the camera.
 * \param tracker	the tracker to use the determine the position of
 *			the guide star. Phase correlation trackers can be
 *			used when there is no suitable guide star
 * \param gpinterval	the interval for guiding actions by the guider port
 * \param aointerval	the interval for actions by the adaptive optics unit
 */
void	Guider::startGuiding(TrackerPtr tracker, double gpinterval,
		double aointerval, bool stepping) {
	// create a TrackingProcess instance
	_state.startGuiding();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new tracking process");
	TrackingProcess	*tp = new TrackingProcess(this, tracker,
		guiderPortDevice, adaptiveOpticsDevice, database());
	trackingprocess = BasicProcessPtr(tp);

	// set the guiding intervals
	if (gpinterval < aointerval) {
		gpinterval = aointerval;
	}
	if (gpinterval < 5) {
		debug(LOG_WARNING, DEBUG_LOG, 0,
			"GP interval is very short: %.3fs, are you sure?",
			gpinterval);
	}
	tp->guiderportInterval(gpinterval);
	tp->adaptiveopticsInterval(aointerval);
	tp->stepping(stepping);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using gp=%.3fs, ao=%.3fs interval",
		gpinterval, aointerval);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "now start tracking");
	trackingprocess->start();
}

/**
 * \brief stop the guiding process
 */
void	Guider::stopGuiding() {
	trackingprocess->stop();
	_state.stopGuiding();
}

/**
 * \brief wait for the guiding process to terminate
 */
bool	Guider::waitGuiding(double timeout) {
	return trackingprocess->wait(timeout);
}

/**
 * \brief retrieve the interval from the guider process
 */
double	Guider::getInterval() {
	TrackingProcess	*tp
		= dynamic_cast<TrackingProcess *>(&*trackingprocess);
	if (NULL == tp) {
		return 10;
	}
	return tp->guiderportInterval();
}

/**
 * \brief retrieve the tracking summary from the 
 */
const TrackingSummary&	Guider::summary() {
	TrackingProcess	*tp
		= dynamic_cast<TrackingProcess *>(&*trackingprocess);
	if (NULL == tp) {
		std::string	cause = stringprintf("wrong state for summary: "
			"%s", Guide::state2string(_state).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw BadState(cause);
	}
	return tp->summary();
}

/**
 * \brief check the current state
 *
 * This method should always be called before the state is check. It checks
 * whether there is a current calibration or guiding process present, and
 * whether it is still running. If it has terminated, the state is updated
 * to reflect the state.
 */
void	Guider::checkstate() {
	Guide::state	s = _state;
	switch (s) {
	case Guide::unconfigured:
		break;
	case Guide::idle:
		break;
	case Guide::calibrating:
		break;
	case Guide::calibrated:
		break;
	case Guide::guiding:
		break;
	}
}

/**
 * \brief Retrieve information about last activation
 */
void Guider::lastAction(double& actiontime, Point& offset, Point& activation) {
	TrackingProcess	*tp
		= dynamic_cast<TrackingProcess *>(&*trackingprocess);
	if (NULL == tp) {
		throw BadState("not currently guiding");
	}
	TrackingPoint	last = tp->last();
	actiontime = last.t;
	offset = last.trackingoffset;
	activation = last.correction;
}

/**
 * \brief Retrieve a descriptor
 */
GuiderDescriptor	Guider::getDescriptor() const {
	return GuiderDescriptor(name(), instrument(), ccdname(),
		guiderportname(), adaptiveopticsname());
}

/**
 * \brief Handle exception callback
 */
void	Guider::callback(const std::exception& /* ex */) {
	Guide::state	s = _state;
	switch (s) {
	case Guide::unconfigured:
	case Guide::idle:
	case Guide::calibrated:
		break;
	case Guide::calibrating:
		// calibraton failed, so we return to the calibrated state
		// if there are calibrations, or to the idle state, if there
		// aren't any calibrations
		// XXX implementation needed
		//_state.configure();
		break;
	case Guide::guiding:
		// guiding failed, return to the configured state
		//_state.stopGuiding();
		break;
	}
}

} // namespace guiding
} // namespace astro
