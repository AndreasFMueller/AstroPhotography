/*
 * GuiderCal.cpp -- Calibration related guider class methods
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

namespace astro {
namespace guiding {

/**
 * \brief update progress value
 */
void	Guider::calibrationProgress(double p) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "PROGRESS %f", p);
	_progress = p;
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
 *
 * The gridpixels suggestion is stored as a parameter to the control
 * device, it is the responsibility of the control device to actually
 * read and use it.
 *
 * \param type		the type of control device
 * \param tracker	the imaging/tracking device
 * \param gridpixels	the suggested grid pixel size to use for the calibration
 * \param east		telescope position
 */
int	Guider::startCalibration(ControlDeviceType type, TrackerPtr tracker,
		float gridpixels, bool east) {
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
		guidePortDevice->setParameter("gridpixels", gridpixels);
		guidePortDevice->setParameter("telescope_east",
			(east) ? 1 : 0);
		return guidePortDevice->startCalibration(tracker);
	}

	if ((type == AO) && adaptiveOpticsDevice) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start AO calibration");
		_state.startCalibrating();
		adaptiveOpticsDevice->setParameter("gridpixels", gridpixels);
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
 *
 * \param calid			the id of the calibration
 * \param meridian_flipped	whether to meridian flip the calibration
 */
void	Guider::useCalibration(int calid, bool meridian_flipped) {
	if (!_state.canAcceptCalibration()) {
		throw BadState("cannot accept calibration now");
	}
	CalibrationStore	store(database());
	if (store.contains(calid, GP)) {
		_state.addCalibration();
		guidePortDevice->calibrationid(calid, meridian_flipped);
		return;
	}
	if (store.contains(calid, AO)) {
		_state.addCalibration();
		adaptiveOpticsDevice->calibrationid(calid, meridian_flipped);
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

} // namespace guiding
} // namespace astro
