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
 * \brief start guiding with the given interval
 */
void	Guider_impl::startGuiding(::CORBA::Float guidinginterval) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding with interval %f",
		guidinginterval);

	// create a set of callbacks, image callback first
	_guider->newimagecallback = astro::callback::CallbackPtr(
		new GuiderImageCallback(*this));

	// Tracking points second
	TrackingInfoCallback	*tric = new TrackingInfoCallback(*this);
	guidingrunid = tric->guidingrunid();
	_guider->trackingcallback = astro::callback::CallbackPtr(tric);

	// Construct the tracker. The rectangle is a rectangle the size of the
	astro::guiding::TrackerPtr	tracker = getTracker();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tracker constructed: %s",
		tracker->toString().c_str());

	// start calibration.
	_guider->startGuiding(tracker, guidinginterval);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiding started");
}

/**
 * \brief get the guiding interval
 */
::CORBA::Float	Guider_impl::getGuidingInterval() {
	return 0;
}

/**
 * \brief stop the guiding process
 */
void	Guider_impl::stopGuiding() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop guiding");
	_guider->stopGuiding();

	// inform the monitors that we have stopped
	tracking_stop();

	// destroy the callbacks
	_guider->newimagecallback = NULL;
	_guider->trackingcallback = NULL;
}

/**
 * \brief Retrieve the most recent point found by the tracker
 */
Astro::TrackingPoint	Guider_impl::mostRecentTrackingPoint() {
	// verify that we really are guiding right now
	if (astro::guiding::guiding != _guider->state()) {
		throw BadState("not currently guiding");
	}

	// ok, we are guiding. Prepare a result structure
	Astro::TrackingPoint	result;
	// So we query the guider for the contents of this structure
	double	lastaction;
	astro::Point	offset;
	astro::Point	activation;
	_guider->lastAction(lastaction, offset, activation);
	result.timeago = astro::Timer::gettime() - lastaction;
	result.trackingoffset.x = offset.x();
	result.trackingoffset.y = offset.y();
	result.activation.x = activation.x();
	result.activation.y = activation.y();

	// that's it, we are read, return the structure
	return result;
}

/**
 * \brief Retrieve the Tracking history of a guide run
 *
 * \param guiderunid	The id of the guide run for which we request
 *			the history. The value -1 means that we want
 *			to retrieve the currently running guide run
 */
Astro::TrackingHistory	*Guider_impl::getTrackingHistory(int guiderunid) {
	if (guiderunid < 0) {
		// verify that we really are guiding right now
		if (astro::guiding::guiding != _guider->state()) {
			throw BadState("not currently guiding");
		}
		guiderunid = guidingrunid;
	}

	// prepare result
	return getTrackingHistory(guiderunid);
}

/**
 * \brief Register a tracking info monitor
 *
 * The Guider_impl class keeps the registered monitors in a map with the
 * monitor id as key. Registering a monitor means creating a new monitor id
 * never before used and putting the TrackingMonitor reference into the map
 * under this new id.
 *
 * Note the name caused by "register" being a reserved wird in C++.
 */
::CORBA::Long	Guider_impl::registerMonitor(::Astro::TrackingMonitor_ptr monitor) {
	return trackinginfochannel.subscribe(monitor);
}

/**
 * \brief Unregister a monitor id
 *
 * \param monitorid	This is the monitor id returned by the register call
 */
void	Guider_impl::unregisterMonitor(::CORBA::Long monitorid) {
	trackinginfochannel.unsubscribe(monitorid);
}

/**
 * \brief update distribution function
 *
 * This method sends the tracking info update to all registered tracking
 * monitors. However, if a monitor fails, it is removed and has to reregister.
 */
void	Guider_impl::update(const Astro::TrackingPoint& trackinginfo) {
	trackinginfochannel.update(trackinginfo);
}

void	Guider_impl::tracking_stop() {
	trackinginfochannel.stop();
}

} // namespace Astro
