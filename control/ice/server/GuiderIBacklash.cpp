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
void    GuiderI::registerBacklashMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register an image callback");
	calibrationimagecallbacks.registerCallback(imagecallback, current);
}

/**
 * \brief Unregister a callback for backlash information
 */
void    GuiderI::unregisterBacklashMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	calibrationimagecallbacks.unregisterCallback(imagecallback, current);
}

/**
 * \brief Start backlash process
 */
void	GuiderI::startBacklash(double interval,
		const Ice::Current& /* current */) {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start backlash");
		astro::guiding::TrackerPtr	tracker = getTracker();
		guider->startBacklash(tracker, interval);
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
	astro::guiding::BacklashDataPtr	data = guider->backlashData();
	return convert(*data);
}

#if 0
/**
 * \brief start imaging with a given exposure
 */
void	GuiderI::startImaging(const Exposure& exposure,
                                const Ice::Current& /* current */) {
	try {
		astro::camera::Exposure	e = convert(exposure);
		guider->startImaging(e);
	} catch (const std::exception& x) {
		BadState	exception;
		exception.cause = std::string(x.what());
		throw exception;
	}
}

/**
 * \brief retrieve the image
 */
ImagePrx	GuiderI::getImage(const Ice::Current& current) {
	// retrieve image
	astro::image::ImagePtr	image = guider->getImage();
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
 * \brief Update information about the image
 */
void	GuiderI::calibrationImageUpdate(
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrationImageUpdate called");
	calibrationimagecallbacks(data);
}

#endif

} // namespace snowstar
