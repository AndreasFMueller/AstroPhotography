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
Guider::Guider(CameraPtr camera, CcdPtr ccd, GuiderPortPtr guiderport,
	Database database)
	: _camera(camera), _guiderport(guiderport), _imager(ccd),
	  _database(database) {
	// default exposure settings
	exposure().exposuretime(1.);
	exposure().frame(ccd->getInfo().getFrame());

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
int	Guider::startCalibration(TrackerPtr tracker, double focallength,
		double pixelsize) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startCalibration(tracker = %s, "
		"focallength = %f, pixelsize = %f)",
		tracker->toString().c_str(), focallength, pixelsize);

	// cleanup any old calibration process
	calibrationCleanup();

	// go into the calibrating state
	_state.startCalibrating();
	
	// first check whether there already is a calibration process
	// running
	if (calibrationprocess) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"calibration already in progress: %f",
			calibrationProgress());
		throw std::runtime_error("calibration already in progress");
	}

	// now create a new calibration process
	calibrationprocess = CalibrationProcessPtr(
		new CalibrationProcess(*this, tracker, _database));

	// start the calibration. This will launch the separate 
	calibrationprocess->calibrate(focallength, pixelsize);

	// register the new calibration in the database
	_calibrationid = 0;
	if (_database) {
		// prepare data for the calibration recrod
		Calibration	calibration;
		calibration.camera = cameraname();
		calibration.ccdid = ccdid();
		calibration.guiderport = guiderportname();
		time(&calibration.when);
		for (int i = 0; i < 6; i++) { calibration.a[i] = 0; }

		// add the record to the database
		CalibrationRecord	record(0, calibration);
		CalibrationTable	calibrationtable(_database);
		_calibrationid = calibrationtable.add(record);
	}
	return _calibrationid;
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
	float	_pixelsize = (info.pixelwidth() + info.pixelheight()) / 2.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %fum", 1000000 * _pixelsize);
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
	guiderprocess = GuiderProcessPtr(new GuiderProcess(*this, interval));
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
	std::string	cameraname = _camera->name().toString();
	std::string	guidername = _guiderport->name().toString();
	int	ccdid = getCcdInfo().getId();
	return GuiderDescriptor(cameraname, ccdid, guidername);
}

} // namespace guiding
} // namespace astro