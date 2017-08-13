/*
 * GuiderIBacklash.cpp -- guider backlash method implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderI.h>
#include <IceConversions.h>
#include <CameraI.h>
#include <CcdI.h>
#include <GuidePortI.h>
#include <ImagesI.h>
#include <AstroGuiding.h>
#include <AstroConfig.h>
#include "CalibrationSource.h"
#include <AstroEvent.h>
#include <ImageDirectory.h>

namespace snowstar {

/**
 * \brief Register a callback for backlash information
 */
void    GuiderI::registerBacklashMonitor(const Ice::Identity& backlashcallback,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register a backlash callback");
	backlashmonitorcallbacks.registerCallback(backlashcallback, current);
}

/**
 * \brief Unregister a callback for backlash information
 */
void    GuiderI::unregisterBacklashMonitor(const Ice::Identity& backlashcallback,
		const Ice::Current& current) {
	backlashmonitorcallbacks.unregisterCallback(backlashcallback, current);
}

/**
 * \brief Start backlash process
 */
void	GuiderI::startBacklash(double interval, BacklashDirection direction,
		const Ice::Current& /* current */) {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start backlash");
		astro::guiding::TrackerPtr	tracker = getTracker();
		guider->startBacklash(tracker, interval, convert(direction));
	} catch (const std::exception& x) {
		BadState	exception;
		exception.cause = std::string(x.what());
		throw exception;
	}
}

/**
 * \brief Stop backlash process
 */
void	GuiderI::stopBacklash(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop the backlash process");
	guider->stopBacklash();
}

/**
 * \brief Send an update to the registered callbacks
 */
void	GuiderI::backlashUpdate(astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backlashUpdate called");
	backlashmonitorcallbacks(data);
}

/**
 * \brief Get the backlash data for the current backlash run
 */
BacklashData	GuiderI::getBacklashData(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backlash data call");
	return convert(guider->backlashData());
}


template<>
void	callback_adapter<BacklashMonitorPrx>(BacklashMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"callback_adapter<BacklashMonitorPrx> called");
	if (!data) {
		return;
	}
		
	// Handle case of a backlash point
	astro::guiding::CallbackBacklashPoint   *backlashpoint
		= dynamic_cast<astro::guiding::CallbackBacklashPoint *>(&*data);
	if (NULL != backlashpoint) {
		if (backlashpoint->data().id < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "negative id, stopping");
			p->stop();
			return;
		}
		BacklashPoint	bp = convert(backlashpoint->data());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "send a point %s",
			backlashpoint->data().toString().c_str());
		p->updatePoint(bp);
		return;
	}

	// handle case of backlash result
	astro::guiding::CallbackBacklashResult   *backlashresult
		= dynamic_cast<astro::guiding::CallbackBacklashResult *>(&*data);
	if (NULL != backlashresult) {
		BacklashResult	r = convert(backlashresult->data());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "send a result %s",
			backlashresult->data().toString().c_str());
		p->updateResult(r);
		return;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown type data type: %s",
		typeid(*data).name());
}


} // namespace snowstar
