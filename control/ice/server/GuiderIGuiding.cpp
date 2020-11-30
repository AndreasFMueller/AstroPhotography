/*
 * GuiderI.cpp -- guider servern implementation
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
 * \brief Start guiding
 */
void GuiderI::startGuiding(Ice::Float gpinterval, Ice::Float aointerval,
		bool stepping,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"start guiding with interval gp=%.1f, ao=%.1f",
		gpinterval, aointerval);
	// construct a tracker
	astro::guiding::TrackerPtr	tracker = getTracker();
	if (!tracker) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no tracker returned");
		throw std::runtime_error("no tracker returned");
	}

	// start guiding
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guiding (filter method %d, tracker %p)",
		_filter_method, tracker.get());
	guider->startGuiding(tracker, gpinterval, aointerval, stepping,
		_filter_method);
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::GUIDE,
		astro::stringprintf("start guiding %s",
		guider->instrument().c_str()));
}

/**
 * \brief Get the gain from the guider
 */
float	GuiderI::getFilterParameter(int dir, const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->filter_parameter(dir);
}

/**
 * /brief Set the gain for a particular direction
 */
void	GuiderI::setFilterParameter(int dir, float value,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got new filter parameter %d: %f",
		dir, value);
	guider->filter_parameter(dir, value);
}

/**
 * \brief Retrieve the guiding interval from the guider
 */
Ice::Float GuiderI::getGuidingInterval(const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->getInterval();
}

/**
 * \brief Stop guiding
 */
void GuiderI::stopGuiding(const Ice::Current& current) {
	CallStatistics::count(current);
	guider->stopGuiding();

	// send the clients that guiding was stopped
	trackingcallbacks.stop();
	//imagecallbacks.stop();

	// remove the callback
	//guider->trackingcallback(NULL);
	astro::event(EVENT_CLASS, astro::events::INFO,
		astro::events::Event::GUIDE,
		astro::stringprintf("stop guiding %s",
		guider->instrument().c_str()));
}

/**
 * \brief Get the most recent image
 */
ImagePrx GuiderI::mostRecentImage(const Ice::Current& current) {
	CallStatistics::count(current);
	// retrieve image
	astro::image::ImagePtr	image = guider->mostRecentImage();
	if (!image) {
		throw NotFound("no image available");
	}

	// store image in image directory
	astro::image::ImageDirectory	imagedirectory;
	std::string	filename = imagedirectory.save(image);

	// return a proxy for the image
	return snowstar::getImage(filename, image->pixel_type(), current);
}

/**
 * \brief Get the most recent tracking point
 */
TrackingPoint GuiderI::mostRecentTrackingPoint(const Ice::Current& current) {
	CallStatistics::count(current);
	if (astro::guiding::Guide::guiding != guider->state()) {
		throw BadState("not currently guiding");
	}

	// get info from the guider
	double	lastaction;
	astro::Point	offset;
	astro::Point	activation;
	guider->lastAction(lastaction, offset, activation);
	
	// construct a tracking point
	TrackingPoint	result;
	result.timeago = converttime(lastaction);
	result.trackingoffset = convert(offset);
	result.activation = convert(activation);
	return result;
}

/**
 * \brief Get the complete tracking history
 */
TrackingHistory GuiderI::getTrackingHistory(Ice::Int id,
	const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get tracking history %d", id);
	astro::guiding::TrackingStore	store;
	return convert(store.get(id));
}

/**
 * \brief Get a tracking history for a given control type
 */
TrackingHistory GuiderI::getTrackingHistoryType(Ice::Int id,
	ControlType type, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get tracking history %d", id);
	astro::guiding::TrackingStore	store;
	switch (type) {
	case ControlGuidePort:
		return convert(store.get(id,
			astro::guiding::GP));
	case ControlAdaptiveOptics:
		return convert(store.get(id,
			astro::guiding::AO));
	}
	debug(LOG_ERR, DEBUG_LOG, 0,
		"control type is invalid (should not happen)");
	throw BadState("not a valid control type");
}

/**
 * \brief Register a callback for monitoring the tracking
 */
void    GuiderI::registerTrackingMonitor(const Ice::Identity& trackingcallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register tracking callback");
	try {
		trackingcallbacks.registerCallback(trackingcallback, current);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register tracking callback: %s %s",
			astro::demangle_string(x).c_str(), x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot register tracking callback for unknown reason");
	}
}

/**
 * \brief Unregister a callback for monitoring the tracking
 */
void    GuiderI::unregisterTrackingMonitor(const Ice::Identity& trackingcallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	trackingcallbacks.unregisterCallback(trackingcallback, current);
}

/**
 * \brief Handle a tracking update
 *
 * This method needs to be called by the callback installed in the guider
 */
void	GuiderI::trackingUpdate(const astro::callback::CallbackDataPtr data) {
	// tell the clients that data has changed
	trackingcallbacks(data);
}

/**
 * \brief Handle a new image from the tracking process
 */
void	GuiderI::trackingImageUpdate(const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "trackingImageUpdate called");

	if (imagerepo()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "send image to repository %s",
			repositoryname().c_str());
		astro::callback::ImageCallbackData	*imageptr
			= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
		if (NULL == imageptr) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"ignoring non-ImageCallbackData");
		} else {
			// save the image in the repository
			imagerepo()->save(imageptr->image());
		}
	}

	// sending data to all registered callbacks
	imagecallbacks(data);
}

/**
 * \brief Handle the summary retrieval method
 */
TrackingSummary	GuiderI::getTrackingSummary(const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calling for tracking summary");
	if (astro::guiding::Guide::guiding != guider->state()) {
		BadState	exception;
		exception.cause = astro::stringprintf("guider is not wrong "
			"state %s", astro::guiding::Guide::state2string(
			guider->state()).c_str());
		throw exception;
	}
	return convert(guider->summary());
}

/**
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<TrackingMonitorPrx>(TrackingMonitorPrx p,
		const astro::callback::CallbackDataPtr data) {
	// check whether the info we got is 
	astro::guiding::TrackingPoint   *trackinginfo
		= dynamic_cast<astro::guiding::TrackingPoint *>(&*data);

	// leave immediately, if there is not Tracking info
	if (NULL == trackinginfo) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not a tracking info object");
		return;
	}

	// tracking 
	TrackingPoint	trackingpoint = convert(*trackinginfo);
	p->update(trackingpoint);
}

/**
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<ImageMonitorPrx>(ImageMonitorPrx p,
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image callback called");
	// first check whether we really got an image
	astro::callback::ImageCallbackData	*imageptr
		= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
	if (NULL == imageptr) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring non-ImageCallbackData");
		return;
	}

	// source image
	astro::image::ImagePtr	source = imageptr->image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback image has size %s",
		source->size().toString().c_str());

	// convert the image into an ImageBuffer
	astro::image::ImageBuffer	buffer(source,
						astro::image::Format::FITS);
	
	// now that the image has been created, send it to the callback
	p->update(*convert(buffer));
}

/**
 * \brief Set the filtering method
 */
void	GuiderI::setFilterMethod(FilterMethod filtermethod,
		const Ice::Current& current) {
	CallStatistics::count(current);
	switch (filtermethod) {
	case FilterNONE:
		_filter_method = astro::guiding::FilterNONE;
		break;
	case FilterGAIN:
		_filter_method = astro::guiding::FilterGAIN;
		break;
	case FilterKALMAN:
		_filter_method = astro::guiding::FilterKALMAN;
		break;
	}
}

/**
 * \brief Get the filtering method
 */
FilterMethod	GuiderI::getFilterMethod(const Ice::Current& current) {
	CallStatistics::count(current);
	switch (_filter_method) {
	case astro::guiding::FilterNONE:	return FilterNONE;
	case astro::guiding::FilterGAIN:	return FilterGAIN;
	case astro::guiding::FilterKALMAN:	return FilterKALMAN;
	}
	throw std::runtime_error("unknown filter method");
}


/**
 * \brief Set a new dither offset
 */
void	GuiderI::setDither(const Point& point,
		const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		guider->dither(convert(point));
	} catch (const astro::guiding::BadState& exception) {
		throw BadState(exception.what());
	}
}

/**
 * \brief Get the current dither offset
 */
Point	GuiderI::getDither(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		return convert(guider->dither());
	} catch (const astro::guiding::BadState& exception) {
		throw BadState(exception.what());
	}
}

/**
 * \brief Generate and set a new dither offset
 */
void    GuiderI::setDitherArcsec(double arcsec, const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		guider->ditherArcsec(arcsec);
	} catch (const astro::guiding::BadState& exception) {
		throw BadState(exception.what());
	}
}


} // namespace snowstar
