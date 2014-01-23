/*
 * Guider_impl.cpp -- implementation of the guider servant
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <Camera_impl.h>
#include <Ccd_impl.h>
#include <GuiderPort_impl.h>
#include <ServantBuilder.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <ImageObjectDirectory.h>
#include <TrackingPersistence.h>
#include <Conversions.h>
#include <GuiderImageCallback.h>
#include <TrackingInfoCallback.h>
#include <CalibrationPointCallback.h>
#include <GuiderFactory_impl.h>

extern astro::persistence::Database	database;

namespace Astro {

/**
 * \brief Retrieve the calibration from the guider
 */
Calibration	*Guider_impl::getCalibration() {
	return Astro::getCalibration(calibrationid);
}

/**
 * \brief Use the this calibration
 */
void	Guider_impl::useCalibration(CORBA::Long id) {
// XXX implementation missing
#if 0
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"set calibration [ %.3f, %.3f, %.3f; %.3f, %.3f, %.3f ]",
		cal.coefficients[0], cal.coefficients[1], cal.coefficients[2],
		cal.coefficients[3], cal.coefficients[4], cal.coefficients[5]
	);
	_guider->calibration(astro::convert(cal));
#endif
}

/**
 * \brief start calibrating
 *
 * Starting the calibration means we also creat a new entry in the calibration
 * table. We do this by installing a CalibrationCallback instance
 */
void	Guider_impl::startCalibration(::CORBA::Float focallength) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start calibration with focal length %f",
		focallength);

	// prepare an image callback
	_guider->newimagecallback = astro::callback::CallbackPtr(
		new GuiderImageCallback(*this));

	// prepare a calibration callback so that the results of the calibration
	// points get recorded in the database
	CalibrationPointCallback	*calcb = new CalibrationPointCallback(*this);
	calibrationid = calcb->calibrationid();
	_guider->calibrationcallback = astro::callback::CallbackPtr(calcb);

	// get the pixel size from the guider's ccd
	astro::camera::CcdInfo	info = _guider->ccd()->getInfo();
	float	pixelsize = (info.pixelwidth() + info.pixelheight()) / 2.;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixelsize: %fum", 1000000 * pixelsize);
	
	// get the tracker
	astro::guiding::TrackerPtr	tracker = getTracker();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker constructed: %s",
		tracker->toString().c_str());

	// start calibration.
	_guider->startCalibration(tracker, focallength, pixelsize);
}

/**
 * \brief stop the calibration process
 */
void	Guider_impl::cancelCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel calibration");
	_guider->cancelCalibration();
}

/**
 * \brief wait for the calibration to complete
 */
bool	Guider_impl::waitCalibration(double timeout) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for calibration to to complete");
	return _guider->waitCalibration(timeout);
}

/**
 * \brief retrieve the progress info
 */
double	Guider_impl::calibrationProgress() {
	double	progress = _guider->calibrationProgress();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check calibration progress: %f",
		progress);
	return progress;
}

/**
 * \brief register a calibration monitor
 */
::CORBA::Long	Guider_impl::registerCalibrationMonitor(
			CalibrationMonitor_ptr monitor) {
	return calibrationchannel.subscribe(monitor);
}

/**
 * \brief unregister a calibration monitor
 */
void	Guider_impl::unregisterCalibrationMonitor(::CORBA::Long monitorid) {
	calibrationchannel.unsubscribe(monitorid);
}

/**
 * \brief stop monitor
 */
void	Guider_impl::calibration_stop() {
	calibrationchannel.stop();
}

/**
 * \brief Inform clients about a new calibration point
 */
void	Guider_impl::update(const Astro::CalibrationPoint& calibrationpoint) {
	calibrationchannel.update(calibrationpoint);
}

} // namespace Astro
