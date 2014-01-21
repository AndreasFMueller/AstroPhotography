/*
 * TrackingInfoCallback.cpp -- Callback for tracking info
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <Camera_impl.h>
#include <Ccd_impl.h>
#include <GuiderPort_impl.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <TrackingPersistence.h>
#include <Conversions.h>
#include <TrackingInfoCallback.h>

extern astro::persistence::Database	database;

namespace Astro {

/**
 * \brief create a TrackingInfCallback
 */
TrackingInfoCallback::TrackingInfoCallback(Guider_impl& guider)
	: _guider(guider) {
	// prepare the guiding run data
	astro::guiding::GuidingRun	guidingrun;
	guidingrun.camera = guider.getCameraName();
	guidingrun.ccdid = guider.getCcdid();
	guidingrun.guiderport = guider.getGuiderPortName();
	time(&guidingrun.whenstarted);

	// create a record from the data object
	astro::guiding::GuidingRunRecord	record(0, guidingrun);

	// add the record to the table
	astro::guiding::GuidingRunTable	guidingruntable(database);
	_guidingrunid = guidingruntable.add(record);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new tracking run with id %d",
		_guidingrunid);
}

/**
 * \brief Process a tracking info update
 */
astro::callback::CallbackDataPtr TrackingInfoCallback::operator()(
	astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new tracking info");
	astro::guiding::TrackingPoint	*trackinginfo
		= dynamic_cast<astro::guiding::TrackingPoint *>(&*data);

	// leave immediately, if there is not Tracking info
	if (NULL == trackinginfo) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"not a tracking info data");
		return data;
	}

	// update the guider, this will send the tracking info to the
	// registered clients
	_guider.update(astro::convert(*trackinginfo));

	// add an entry to the database
	astro::guiding::TrackingPointRecord	tracking(0, _guidingrunid,
							*trackinginfo);
	astro::guiding::TrackingTable	trackingtable(database);
	long	tid = trackingtable.add(tracking);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new tracking entry with id %ld", tid);

	return data;
}

} // namespace Astro
