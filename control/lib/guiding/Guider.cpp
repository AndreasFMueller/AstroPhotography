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

using namespace astro::image;
using namespace astro::camera;
using namespace astro::callback;
using namespace astro::persistence;

namespace astro {
namespace guiding {

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
	CcdPtr ccd, GuiderPortPtr guiderport,
	Database database)
	: _instrument(instrument), _guiderport(guiderport), _imager(ccd),
	  _database(database) {
	// default exposure settings
	exposure().exposuretime(1.);
	exposure().frame(ccd->getInfo().getFrame());

	// default focallength
	_focallength = 1;

	// at this point the guider is sufficiently configured, although
	// this configuration is not optimal
	_state.configure();
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
 * \brief Set a calibration
 *
 * If the calibration data is already known, then we can immediately set
 * the calibration without going through the calibration process each time
 * we build the guider.
 */
void	Guider::calibration(const GuiderCalibration& calibration) {
	_state.addCalibration();
	_calibration = calibration;
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

	// This will implicitely cleanup the calibration process,
	// if there is one. If there is none, this operation will
	// do nothing
	calibrationprocess = NULL;
}

/**
 * \brief start an asynchronous calibration process
 *
 * This method first checks that no other calibration thread is running,
 * and if so, starts a new thread.
 */
int	Guider::startCalibration(TrackerPtr tracker) {
	double	pixelsize = getPixelsize();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startCalibration(tracker = %s, "
		"focallength = %.3fmm, pixelsize = %.2fum)",
		tracker->toString().c_str(), 1000 * _focallength,
		1000000 * pixelsize);

	// cleanup any old calibration process
	calibrationCleanup();

	// go into the calibrating state
	_state.startCalibrating();
	
	// first check whether there already is a calibration process
	// running
	if (calibrationprocess) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"calibration already in progress: %.3f",
			calibrationProgress());
		throw std::runtime_error("calibration already in progress");
	}

	// now create a new calibration process
	calibrationprocess = CalibrationProcessPtr(
		new CalibrationProcess(this, tracker, _database));

	// start the calibration. This will launch the separate 
	calibrationprocess->focallength(_focallength);
	calibrationprocess->pixelsize(pixelsize);
	calibrationprocess->start();

	// register the new calibration in the database
	_calibrationid = 0;
	if (_database) {
		// prepare data for the calibration recrod
		PersistentCalibration	calibration;
		calibration.instrument = instrument();
		calibration.ccd = ccdname();
		calibration.controldevice = guiderportname();
		calibration.focallength = _focallength;
		calibration.masPerPixel
			= (pixelsize / _focallength) * (180*3600*1000 / M_PI);
		time(&calibration.when);
		for (int i = 0; i < 6; i++) { calibration.a[i] = 0; }

		// add the record to the database
		CalibrationRecord	record(0, calibration);
		CalibrationTable	calibrationtable(_database);
		_calibrationid = calibrationtable.add(record);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"saved calibration record id = %d, masPerPixel = %.3f",
			_calibrationid, calibration.masPerPixel);
	}
	return _calibrationid;
}

/**
 * \brief save a guider calibration
 */
void	Guider::saveCalibration(const GuiderCalibration& cal) {
	if (!_database) {
		return;
	}
	CalibrationStore	calstore(_database);
	calstore.saveCalibration(_calibrationid, cal);
}

/**
 * \brief inquire about the current state of the calibration process
 *
 * Find out how far along the calibraition process we are. This 
 */
double	Guider::calibrationProgress() {
	if (_state != Guide::calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	return calibrationprocess->progress();
}

/**
 * \brief cancel a calibration that is still in progress
 */
void	Guider::cancelCalibration() {
	if (_state != Guide::calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	calibrationprocess->stop();
}

/**
 * \brief wait for the calibration to complete
 */
bool	Guider::waitCalibration(double timeout) {
	if (_state != Guide::calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	return calibrationprocess->wait(timeout);
}


/**
 * \brief start an exposure
 */
void	Guider::startExposure() {
	imager().startExposure(exposure());
}

/**
 * \brief get the image
 *
 * Retrieve an image from the imager. Each image is also sent to the
 * newimagecallback, if set
 */
ImagePtr	Guider::getImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getImage() called");
	ImagePtr	image = imager().getImage();
	callbackImage(image);
	mostRecentImage = image;
	return image;
}

/**
 * \brief send the image to the callback
 *
 * This function wraps an image in a ImageCallbackData object and sends
 * it to the newimagecallback, if it is set.
 */
void	Guider::callbackImage(ImagePtr image) {
	if (newimagecallback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending new image to callback");
		ImageCallbackData	*argp = 
			new ImageCallbackData(image);
		CallbackDataPtr	arg(argp);
		newimagecallback->operator()(arg);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback return");
	}
}

/**
 * \brief get a good measure for the pixel size of the CCD
 *
 * This method returns the average of the pixel dimensions, this will
 * give strange values for binned cameras. Binning looks like a strange
 * idea for a guide camera anyway.
 */
double	Guider::getPixelsize() {
	astro::camera::CcdInfo  info = ccd()->getInfo();
	float	_pixelsize = (info.pixelwidth() * _exposure.mode().x()
			+ info.pixelheight() * _exposure.mode().y()) / 2.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %.2fum",
		1000000 * _pixelsize);
	return _pixelsize;
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

/**
 * \brief start tracking
 */
void	Guider::startGuiding(TrackerPtr tracker, double interval) {
	// create a GuiderProcess instance
	_state.startGuiding();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new guider process");
	guiderprocess = GuiderProcessPtr(new GuiderProcess(this, interval,
		_database));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new guider process created");
	guiderprocess->start(tracker);
}

/**
 * \brief stop the guiding process
 */
void	Guider::stopGuiding() {
	guiderprocess->stop();
	_state.stopGuiding();
}

/**
 * \brief wait for the guiding process to terminate
 */
bool	Guider::waitGuiding(double timeout) {
	return guiderprocess->wait(timeout);
}

/**
 * \brief retrieve the interval from the guider process
 */
double	Guider::getInterval() {
	return guiderprocess->interval();
}

/**
 * \brief retrieve the tracking summary from the 
 */
const TrackingSummary&	Guider::summary() {
	if (!guiderprocess) {
		std::string	cause = stringprintf("wrong state for summary: "
			"%s", Guide::state2string(_state).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", cause.c_str());
		throw BadState(cause);
	}
	return guiderprocess->summary();
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
		break;
	case Guide::calibrated:
		break;
	case Guide::guiding:
		if (guiderprocess) {
			if (!guiderprocess->isrunning()) {
				_state.addCalibration();
				guiderprocess = NULL;
			}
		}
		break;
	}
}

/**
 * \brief Retrieve information about last activation
 */
void Guider::lastAction(double& actiontime, Point& offset,
		Point& activation) {
	if (!guiderprocess) {
		throw std::runtime_error("not currently guiding");
	}
	guiderprocess->lastAction(actiontime, offset, activation);
}

/**
 * \brief Retrieve a descriptor
 */
GuiderDescriptor	Guider::getDescriptor() const {
	return GuiderDescriptor(instrument(), ccdname(), guiderportname());
}

} // namespace guiding
} // namespace astro
