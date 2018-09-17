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
#include "CalibrationRedirector.h"
#include "TrackingProcess.h"
#include <Backlash.h>

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
	CcdPtr ccd, GuidePortPtr guideport, AdaptiveOpticsPtr adaptiveoptics,
	Database database)
	: GuiderBase(guidername, ccd, database), _guideport(guideport),
	  _adaptiveoptics(adaptiveoptics) {
	// default exposure settings
	exposure().exposuretime(1.);
	exposure().frame(ccd->getInfo().getFrame());

	// default focallength
	_focallength = 1;
	_guiderate = 0.5;

	// We have to install a callback for calibrations
	CallbackPtr	calcallback(new CalibrationRedirector(this));
	addProgressCallback(calcallback);

	// create control devices
	if (guideport) {
		guidePortDevice = ControlDevicePtr(
			new ControlDevice<GuidePort, GuiderCalibration, GP>(this,
				guideport, database));
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
 * \brief Destructor
 */
Guider::~Guider() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying guider at %p", this);
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
		if ((guidePortDevice) && (guidePortDevice->calibrating())) {
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
		guidePortDevice, adaptiveOpticsDevice, database());
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
	tp->guideportInterval(gpinterval);
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
	return tp->guideportInterval();
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
	case Guide::darkacquire:
		if (!_darkthread->isrunning()) {
			_state.endDarkAcquire();
		}
		break;
	case Guide::flatacquire:
		if (!_flatthread->isrunning()) {
			_state.endFlatAcquire();
		}
		break;
	case Guide::imaging:
		if (!_imagethread->isrunning()) {
			endImaging(ImagePtr(NULL));
		}
		break;
	case Guide::backlash:
		if (!_backlashthread->isrunning()) {
			_state.endBacklash();
		}
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
	GuiderDescriptor	descriptor(name(), instrument(), ccdname(),
		guideportname(), adaptiveopticsname());
	return descriptor;
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
	case Guide::darkacquire:
		// XXX implementation required
		_state.endDarkAcquire();
		break;
	case Guide::flatacquire:
		// XXX implementation required
		_state.endFlatAcquire();
		break;
	case Guide::imaging:
		// XXX ist this the right implementation?
		endImaging(ImagePtr(NULL));
		break;
	case Guide::backlash:
		_state.endBacklash();
		break;
	}
}

/**
 * \brief get the gain from the guider
 *
 * This method reads the gain from the TrackingProcess, if available,
 * this allows the TrackingProcess to dynamically change the gain if it
 * notices increasing amplitude e.g.
 */
float	Guider::gain(gain_direction dir) {
	TrackingProcess	*tp = NULL;
	if (trackingprocess) {
		tp = dynamic_cast<TrackingProcess*>(&*trackingprocess);
	}
	switch (dir) {
	case GAIN_X:
		if (tp) {
			_gain_x = tp->parameter(0);
		}
		return _gain_x;
	case GAIN_Y:
		if (tp) {
			_gain_y = tp->parameter(1);
		}
		return _gain_y;
	}
}

/**
 * \brief Set the gain for a particular direction
 *
 * If a TrackingProcess is running in the background, we also set the
 * gain in that process.
 */
void	Guider::gain(gain_direction dir, float g) {
	TrackingProcess	*tp = NULL;
	if (trackingprocess) {
		tp = dynamic_cast<TrackingProcess*>(&*trackingprocess);
	}
	switch (dir) {
	case GAIN_X:
		if (tp) {
			tp->parameter(0, g);
		}
		_gain_x = g;
		break;
	case GAIN_Y:
		if (tp) {
			tp->parameter(1, g);
		}
		_gain_y = g;
		break;
	}
}

} // namespace guiding
} // namespace astro
