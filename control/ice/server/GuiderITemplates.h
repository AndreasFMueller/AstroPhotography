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
 * \brief Function to copy image pixels to the SimpleImage structure
 */
template<typename pixel>
void	copy_image(const astro::image::Image<pixel> *source,
		SimpleImage& target, double scale) {
	for (int y = 0; y < target.size.height; y++) {
		for (int x = 0; x < target.size.width; x++) {
			unsigned short	value = scale * source->pixel(x, y);
			target.imagedata.push_back(value);
		}
	}
}

#define copypixels(pixel, scale, source, target)		\
do {								\
	astro::image::Image<pixel>	*im			\
		= dynamic_cast<astro::image::Image<pixel> *>(	\
			&*source);				\
	if (NULL != im) {					\
		copy_image(im, target, scale);			\
	}							\
} while (0)

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

	// convert the image so that it is understood by the
	// ImageMonitor proxy
	SimpleImage	target;
	target.size = convert(source->size());

	// copy pixels to the target structure
	copypixels(unsigned short, 1, source, target);
	copypixels(unsigned char, 256, source, target);
	copypixels(unsigned long, (1. / 65536), source, target);
	copypixels(double, 1, source, target);
	copypixels(float, 1, source, target);
	
	if ((0 == target.size.width) && (0 == target.size.height)) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"don't know how to handle non short images");
		return;
	}

	// now that the image has been created, send it to the callback
	p->update(target);
}

#if 0
/**
 * \brief calback adapter for Calibration monitor
 */
template<>
void	callback_adapter<CalibrationMonitorPrx>(CalibrationMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	// handle a calibration point callback call
	astro::guiding::CalibrationPointCallbackData	*calibrationpoint
		= dynamic_cast<astro::guiding::CalibrationPointCallbackData *>(&*data);
	if (NULL != calibrationpoint) {
		// convert the calibration point into
		CalibrationPoint	point
			= convert(calibrationpoint->data());
		p->update(point);
		return;
	}

	// handle a completed calibration callback call, by sending the stop
	// signal
	astro::guiding::CalibrationCallbackData	*calibration
		= dynamic_cast<astro::guiding::CalibrationCallbackData *>(&*data);
	if (NULL != calibration) {
		p->stop();
		return;
	}
}

/**
 * \brief callback adapter for Image Calibration Monitor
 */
template<>
void	callback_adapter<CalibrationImageMonitorPrx>(CalibrationImageMonitorPrx& p,
		const astro::callback::CallbackDataPtr data) {
	astro::camera::CalibrationImageProgressData	*calimgp
		= dynamic_cast<astro::camera::CalibrationImageProgressData*>(&*data);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Calibration Image callback called");

	if (NULL != calimgp) {
		int	imageno = calimgp->data().imageno;
		int	imagecount = calimgp->data().imagecount;
		if (imageno < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"got end of calibration image run");
			p->stop();
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"calibration image progress: %d/%d",
				imageno, imagecount);
			p->update(convert(calimgp->data()));
		}
		return;
	}

}
#endif

} // namespace snowstar
