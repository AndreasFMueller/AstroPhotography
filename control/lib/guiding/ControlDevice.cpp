/*
 * ControlDevice.cpp -- Control Device specialisations
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <BasicProcess.h>
#include <GPCalibrationProcess.h>
#include <AOCalibrationProcess.h>
#include <algorithm>
#include "GuidePortAction.h"

using namespace astro::callback;

namespace astro {
namespace guiding {

//////////////////////////////////////////////////////////////////////
// Specialization to GuidePort
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::GuidePort,
		GuiderCalibration, GP>::startCalibration(TrackerPtr tracker) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GP calibration start");

	// create the calibration process
	CalibrationProcess	*calibrationprocess
		= new GPCalibrationProcess(_guider, _device, tracker, _database);
	process = BasicProcessPtr(calibrationprocess);

	// start the process and update the record in the database
	return ControlDeviceBase::startCalibration(tracker);
}

/**
 * \brief apply a correction and send it to the GuidePort
 */
template<>
Point	ControlDevice<camera::GuidePort,
		GuiderCalibration, GP>::correct(const Point& point, double Deltat,
			bool stepping) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guideport correction %s, %.2f",
		point.toString().c_str(), Deltat);
	// give up if not configured
	if (!_calibration->complete()) {
		return point;
	}

	// now compute the correction based on the calibration
	Point	correction = _calibration->correction(point, Deltat);

	// apply the correction to the guider port
	debug(LOG_DEBUG, DEBUG_LOG, 0, "apply GP correction: %s",
		correction.toString().c_str());
	double	dt = (Deltat > 0.5) ? (Deltat - 0.5) : 0;
	GuidePortAction	*action = new GuidePortAction(_device,
					correction, dt);
	action->stepping(stepping);
	ActionPtr	aptr(action);
	asynchronousaction.execute(aptr);

	// log the information to the callback
	TrackingPoint	ti;
	ti.t = Timer::gettime();
	ti.trackingoffset = point;
	ti.correction = correction;
	ti.type = GP;
	_guider->callback(ti);

	// no remaining error after a guider port correction ;-)
	return Point(0, 0);
}

//////////////////////////////////////////////////////////////////////
// Specialization to AdaptiveOptics
//////////////////////////////////////////////////////////////////////
template<>
int	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration, AO>::startCalibration(
			TrackerPtr tracker) {

	// create a new calibraiton process
	AOCalibrationProcess	*aocalibrationprocess
		= new AOCalibrationProcess(_guider, _device, tracker,
			_database);
	process = BasicProcessPtr(aocalibrationprocess);

	// start the calibration
	return ControlDeviceBase::startCalibration(tracker);
}

/**
 * \brief Apply correction to adaptive optics device
 */
template<>
Point	ControlDevice<camera::AdaptiveOptics,
		AdaptiveOpticsCalibration, AO>::correct(const Point& point,
			double Deltat, bool /* stepping */) {
	// give up if not configured
	if (!_calibration->complete()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "AO not calibrated");
		return point;
	}

	// now compute the calibration
	Point	correction = _calibration->correction(point, Deltat);

	// get the current correction
	Point	newposition = _device->get() + correction;
	try {
		_device->set(newposition);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set new position %s: %s",
			newposition.toString().c_str(), x.what());
		correction = Point(0, 0);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set new position %s",
			newposition.toString().c_str());
		correction = Point(0, 0);
	}

	// log the information to the callback
	TrackingPoint	ti;
	ti.t = Timer::gettime();
	ti.trackingoffset = point;
	ti.correction = correction;
	ti.type = AO;
	_guider->callback(ti);

	// get the remaining correction
	return _calibration->offset(_device->get() * -1.);
}

} // namespace guiding
} // namespace astro
