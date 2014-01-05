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

using namespace astro::image;
using namespace astro::camera;
using namespace astro::callback;

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
Guider::Guider(CameraPtr camera, CcdPtr ccd, GuiderPortPtr guiderport)
	: _camera(camera), _guiderport(guiderport), _imager(ccd) {
	// default exposure settings
	exposure().exposuretime = 1.;
	exposure().frame = ccd->getInfo().getFrame();

	// at this point the guider is sufficiently configured, although
	// this configuration is not optimal
	_state.configure();
}

/**
 * \brief Retrieve the state 
 *
 * Get the guider state. The guider keeps state information in the guider
 * state machine, so we have to convert that to the GuiderState constants.
 * This is done by the cast operator of the GuiderStateMachine class.
 */
GuiderState	Guider::state() const {
	GuiderState	result = _state;
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
	if (state() == calibrating) {
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
void	Guider::startCalibration(TrackerPtr tracker, double focallength,
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
		new CalibrationProcess(*this, tracker));

	// start the calibration. This will launch the separate 
	calibrationprocess->calibrate(focallength, pixelsize);
}

/**
 * \brief inquire about the current state of the calibration process
 *
 * Find out how far along the calibraition process we are. This 
 */
double	Guider::calibrationProgress() {
	if (_state != calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	return calibrationprocess->progress();
}

/**
 * \brief cancel a calibration that is still in progress
 */
void	Guider::cancelCalibration() {
	if (_state != calibrating) {
		throw std::runtime_error("not currently calibrating");
	}
	calibrationprocess->stop();
}

/**
 * \brief wait for the calibration to complete
 */
bool	Guider::waitCalibration(double timeout) {
	if (_state != calibrating) {
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
		GuiderNewImageCallbackData	*argp = 
			new GuiderNewImageCallbackData(image);
		CallbackDataPtr	arg(argp);
		newimagecallback->operator()(arg);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "callback return");
	}
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
 * \brief check the current state
 *
 * This method should always be called before the state is check. It checks
 * whether there is a current calibration or guiding process present, and
 * whether it is still running. If it has terminated, the state is updated
 * to reflect the state.
 */
void	Guider::checkstate() {
	GuiderState	s = _state;
	switch (s) {
	case unconfigured:
		break;
	case idle:
		break;
	case calibrating:
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
	case calibrated:
		break;
	case guiding:
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

} // namespace guiding
} // namespace astro
