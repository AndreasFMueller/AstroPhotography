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
#include <ServerDatabase.h>
#include <AstroFormat.h>

namespace Astro {

/**
 * \brief Retrieve the calibration from the guider
 */
Calibration	*Guider_impl::getCalibration() {
	return ServerDatabase().getCalibration(calibrationid);
}

/**
 * \brief Use the this calibration
 *
 * This method retrieves a calibration record from the database and installs
 * it as the current calibration in the guider. This allows to reuse previously
 * recorded calibrations. The calibration is identified by its id. If a negative
 * id is specified, the most recent calibration matching the guider descriptor
 * is used.
 * \param id	id of the calibration to install
 */
void	Guider_impl::useCalibration(CORBA::Long id) {
	// get the the database
	astro::persistence::Database	database = ServerDatabase().database();

	// get the calibration table
	astro::guiding::CalibrationTable	table(database);

	// if the id is negative, then we should use the most recent
	// calibration we can find
	if (id < 0) {
		// retrieve a list of calibrations done with this guider
		astro::guiding::GuiderDescriptor	descriptor
			= _guider->getDescriptor();
		std::list<long>	idlist = table.selectids(descriptor);

		// if there are none, we throw an exception
		if (idlist.size() == 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no calibration for %s",
				descriptor.toString().c_str());
			throw NotFound("no calibration for this guider");
		}

		// take the last entry
		id = idlist.back();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "last calibration id: %d", id);
	}

	// retrieve calibration data from the database
	astro::guiding::CalibrationRecord	record = table.byid(id);

	// copy the data into the calibration object
	astro::guiding::GuiderCalibration	cal;
	for (int i = 0; i < 6; i++) {
		cal.a[i] = record.a[i];
	}
	calibrationid = id;

	// set the calibration
	_guider->calibration(cal);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"set calibration %d: [ %.3f, %.3f, %.3f; %.3f, %.3f, %.3f ]",
		calibrationid, cal[0], cal[1], cal[2], cal[3], cal[4], cal[5]);
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
	CalibrationPointCallback	*calcb
		= new CalibrationPointCallback(*this);
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
