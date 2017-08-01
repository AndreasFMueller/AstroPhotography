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
	if ((type == GP) && guidePortDevice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start GuidePort calibration");
		_state.startCalibrating();
		guidePortDevice->setParameter("focallength", focallength());
		guidePortDevice->setParameter("guiderate", guiderate());
		return guidePortDevice->startCalibration(tracker);
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
void	Guider::saveCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accepting completed calibration");
	if (!_state.canAcceptCalibration()) {
		return;
	}
	checkCalibrationState();
}

/**
 * \brief Forget a calibration
 *
 * This method is called by the control device or the calibration process
 * when a calibration fails. Since the information is already in the database
 * (the calibration remains incomplete), we only have to adjust the state.
 *
 */
void	Guider::forgetCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "forgetting incomplete calibration");
	if (!_state.canFailCalibration()) {
		return;
	}
	checkCalibrationState();
}

/**
 * \brief Check the current calibrations tate
 *
 * The guider is calibrated if one of its control devices is calibrated.
 * This makes it a little more difficult to determine the guider state
 * after a calibration completes or fails. Since completion and failure
 * use the same logic, this is collected in this method.
 */
void	Guider::checkCalibrationState() {
	// we have received a calibration, lets see what this means
        bool	something_calibrated = false;
	if (adaptiveOpticsDevice) {
		something_calibrated |= adaptiveOpticsDevice->iscalibrated();
	}
	if (guidePortDevice) {
		something_calibrated |= guidePortDevice->iscalibrated();
	}
	if (something_calibrated) {
		_state.addCalibration();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Guider now calibrated");
	} else {
		_state.failCalibration();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Guider uncalibrated");
	}
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
		guidePortDevice->calibrationid(calid);
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
		guidePortDevice->calibrationid(-1);
		break;
	case AO:
		adaptiveOpticsDevice->calibrationid(-1);
		break;
	}

	// if neither device is no calibrated, go into the idle state
	if ((!guidePortDevice->iscalibrated())
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
	if (guidePortDevice) {
		if (guidePortDevice->calibrating()) {
			guidePortDevice->cancelCalibration();
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
	if (guidePortDevice) {
		if (guidePortDevice->calibrating()) {
			return guidePortDevice->waitCalibration(timeout);
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
	case Guide::imaging:
		if (!_imagethread->isrunning()) {
			endImaging(ImagePtr(NULL));
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
	case Guide::imaging:
		// XXX ist this the right implementation?
		endImaging(ImagePtr(NULL));
		break;
	}
}

/**
 * \brief Callback class to signal the end of the guide process
 */
class DarkEndCallback : public Callback {
	Guider&	_guider;
public:
	DarkEndCallback(Guider& guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "endDark callback called");
		_guider.endDark();
		return data;
	}
};

/**
 * \brief Start getting a dark image
 *
 * \param exposuretime	exposure time to use for darks
 * \param imagecount	number of images to use to construct the dark
 */
void	Guider::startDark(double exposuretime, int imagecount) {
	_state.startDarkAcquire();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start to acquire a dark");
		// set up the dark work
		_darkwork = DarkWorkImagerPtr(new DarkWorkImager(imager()));
		_darkwork->exposuretime(exposuretime);
		_darkwork->imagecount(imagecount);
		_darkwork->endCallback(CallbackPtr(new DarkEndCallback(*this)));

		// set iup the thread
		DarkWorkImagerThread	*dwit
			= new DarkWorkImagerThread(&*_darkwork);
		_darkthread = DarkWorkImagerThreadPtr(dwit);

		// start the thread
		_darkthread->start();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark acquire is running");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"dark acquisition start failed: %s", x.what());
		callback(x);
	}
}

/**
 * \brief Method to signal the end of the dark acquisition process
 */
void	Guider::endDark() {
	_state.endDarkAcquire();
}

/**
 * \brief Callback class to signal the end of the guide process
 */
class ImageEndCallback : public Callback {
	Guider&	_guider;
public:
	ImageEndCallback(Guider& guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "endImage callback called");
		if (!data) {
			_guider.endImaging(ImagePtr(NULL));
		} else {
			ImageCallbackData	*icd
				= dynamic_cast<ImageCallbackData*>(&*data);
			if (icd) {
				_guider.endImaging(icd->image());
			} else {
				_guider.endImaging(ImagePtr(NULL));
			}
		}
		return data;
	}
};

/**
 * \brief Start acquiring an image via the imager
 *
 * \param exposure	exposure settings to use
 */
void	Guider::startImaging(const Exposure& exposure) {
	_state.startImaging();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start to acquire image %s",
			exposure.toString().c_str());
		_imagework = ImageWorkImagerPtr(
				new ImageWorkImager(imager(), exposure));
		_imagework->endcallback(CallbackPtr(new ImageEndCallback(*this)));
		// set iup the thread
		ImageWorkImagerThread	*iwit
			= new ImageWorkImagerThread(&*_imagework);
		_imagethread = ImageWorkImagerThreadPtr(iwit);

		// start the thread
		_imagethread->start();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "imaging process is running");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "image acquisition failed: %s",
			x.what());
		callback(x);
	}
}

/**
 * \brief Method to signal the end of the image acquisition process
 */
void	Guider::endImaging(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received an image");
	_image = image;
	if (_image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Image size: %s",
			_image->size().toString().c_str());
	}
	_state.endImaging();
}

} // namespace guiding
} // namespace astro
