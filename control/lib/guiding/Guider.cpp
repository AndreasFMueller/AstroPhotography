/*
 * Guider.cpp -- classes implementing guiding
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
#include <CalibrationPersistence.h>
#include <CalibrationStore.h>
#include <TrackingProcess.h>

using namespace astro::image;
using namespace astro::camera;
using namespace astro::callback;
using namespace astro::persistence;

namespace astro {
namespace guiding {

/**
 * \brief Auxiliary class to ensure calibrations found are sent to the guider
 */
class GuiderCalibrationRedirector : public Callback {
	Guider	*_guider;
public:
	GuiderCalibrationRedirector(Guider *guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data);
};


CallbackDataPtr	GuiderCalibrationRedirector::operator()(CallbackDataPtr data) {
	// handle the calibration
	{
		GuiderCalibrationCallbackData	*cal
			= dynamic_cast<GuiderCalibrationCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "calibration update");
			_guider->saveCalibration(cal->data());
		}
	}

	// handle progress updates
	{
		ProgressInfoCallbackData	*cal
			= dynamic_cast<ProgressInfoCallbackData *>(&*data);
		if (NULL != cal) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "progress update");
			_guider->calibrationProgress(cal->data().progress);
			if (cal->data().aborted) {
				_guider->forgetCalibration();
			}
		}
	}

	return data;
}

//////////////////////////////////////////////////////////////////////
// Guider implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct a guider from 
 *
 * Since the guider includes an exposure, it also initializes the exposure
 * to some default values. The default exposure time is 1 and the
 * default frame is the entire CCD area.
 */
Guider::Guider(const std::string& instrument,
	CcdPtr ccd, GuiderPortPtr guiderport, AdaptiveOpticsPtr adaptiveoptics,
	Database database)
	: GuiderBase(instrument, ccd, database), _guiderport(guiderport),
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
			new ControlDevice<GuiderPort, GuiderCalibration>(this,
				guiderport, database));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "guider port control device");
	}
	if (adaptiveoptics) {
		adaptiveOpticsDevice = ControlDevicePtr(
			new ControlDevice<AdaptiveOptics,
				AdaptiveOpticsCalibration>(this,
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
Guide::state	Guider::state() const {
	Guide::state	result = _state;
	return result;
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
int	Guider::startCalibration(BasicCalibration::CalibrationType type,
		TrackerPtr tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration for %s",
		BasicCalibration::type2string(type).c_str());
	// make sure we have a tracker
	if (!tracker) {
		debug(LOG_ERR, DEBUG_LOG, 0, "tracker not defined");
		throw std::runtime_error("tracker not set");
	}

	// are we in the correct state
	if (!_state.canStartCalibrating()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start calibrating");
		throw std::runtime_error("wrong state");
	}
	_progress = 0;

	// start calibration
	if ((type == BasicCalibration::GP) && guiderPortDevice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuiderPort calibration");
		_state.startCalibrating();
		guiderPortDevice->setParameter("focallength", focallength());
		return guiderPortDevice->startCalibration(tracker);
	}

	if ((type == BasicCalibration::AO) && adaptiveOpticsDevice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start AO calibration");
		_state.startCalibrating();
		return adaptiveOpticsDevice->startCalibration(tracker);
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "cannot calibrate, no device");
	throw std::runtime_error("bad state");
}

/**
 * \brief save a guider calibration
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
	}
}

void	Guider::useCalibration(int calid) {
	if (!_state.canAcceptCalibration()) {
		throw std::runtime_error("cannot accept calibration now");
	}
	CalibrationStore	store(database());
	if (store.contains(calid, BasicCalibration::GP)) {
		_state.addCalibration();
		guiderPortDevice->calibrationid(calid);
	}
	if (store.contains(calid, BasicCalibration::AO)) {
		_state.addCalibration();
		adaptiveOpticsDevice->calibrationid(calid);
	}
}

/**
 * \brief cancel a calibration that is still in progress
 */
void	Guider::cancelCalibration() {
	if (_state != Guide::calibrating) {
		throw std::runtime_error("not currently calibrating");
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
		throw std::runtime_error("not currently calibrating");
	}
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
	return true;
}

/**
 * \brief get a default tracker
 *
 * This is not the only possible tracker to use with the guiding process,
 * but it works currently quite well
 */
TrackerPtr	Guider::getTracker(const Point& point) {
	astro::camera::Exposure exp = exposure();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "origin: %s",
		exp.origin().toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "point: %s",
		point.toString().c_str());
	astro::Point    difference = point - exp.origin();
	int	x = difference.x();
	int	y = difference.y();
	astro::image::ImagePoint        trackerstar(x, y);
	astro::image::ImageRectangle    trackerrectangle(exp.size());
	astro::guiding::TrackerPtr      tracker(
		new astro::guiding::StarTracker(trackerstar,
			trackerrectangle, 10));
	return tracker;
}

TrackerPtr	Guider::getPhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a standard phase tracker");
	return TrackerPtr(new PhaseTracker());
}

TrackerPtr	Guider::getDiffPhaseTracker() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a differential phase tracker");
	return TrackerPtr(new DifferentialPhaseTracker());
}

/**
 * \brief start tracking
 */
void	Guider::startGuiding(TrackerPtr tracker, double interval) {
	// create a TrackingProcess instance
	_state.startGuiding();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new tracking process");
	TrackingProcess	*tp = new TrackingProcess(this, tracker,
		guiderPortDevice, adaptiveOpticsDevice, database());
	tp->guiderportInterval(interval);
#if 0
	// XXX temporary, for debugging
	tp->adaptiveopticsInterval(interval + 1);
#endif
	trackingprocess = BasicProcessPtr(tp);

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
#if 0
		if (calibrationprocess) {
			if (!calibrationprocess->isrunning()) {
				if (iscalibrated()) {
					_state.addCalibration();
				} else {
					_state.configure();
				}
				calibrationprocess = NULL;
			}
		}
#endif
		break;
	case Guide::calibrated:
		break;
	case Guide::guiding:
#if 0
		if (guiderprocess) {
			if (!guiderprocess->isrunning()) {
				_state.addCalibration();
				guiderprocess = NULL;
			}
		}
#endif
		break;
	}
}

/**
 * \brief Retrieve information about last activation
 */
void Guider::lastAction(double& actiontime, Point& offset,
		Point& activation) {
	TrackingProcess	*tp
		= dynamic_cast<TrackingProcess *>(&*trackingprocess);
	if (NULL == tp) {
		throw std::runtime_error("not currently guiding");
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

} // namespace guiding
} // namespace astro
