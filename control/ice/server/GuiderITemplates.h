/*
 * GuiderITemplates.cpp -- Guider templates moslty callback adapters
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
 * \brief calback adapter for Tracking monitor
 */
template<>
void	callback_adapter<TrackingMonitorPrx>(TrackingMonitorPrx& p,
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
void	callback_adapter<ImageMonitorPrx>(ImageMonitorPrx& p,
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
	p->update(buffer);
}


} // namespace snowstar
