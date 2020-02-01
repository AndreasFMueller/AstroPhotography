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

/**
 * \brief start the dark acquire process
 */
void	GuiderI::startDarkAcquire(double exposuretime, int imagecount,
		double badpixellimit, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startDarkAcquire called");
	try {
		guider->startDark(exposuretime, imagecount, badpixellimit);
	} catch (const std::exception& x) {
		BadState	exception;
		exception.cause = std::string(x.what());
		throw exception;
	}
}

/**
 * \brief Return the dark image of the imager
 */
ImagePrx	GuiderI::darkImage(const Ice::Current& current) {
	CallStatistics::count(current);
	// retrieve image
	astro::image::ImagePtr	darkimage = guider->imager().dark();
	if (!darkimage) {
		NotFound	exception;
		exception.cause = std::string("no dark image available");
		throw exception;
	}

	// store image in image directory
	astro::image::ImageDirectory	imagedirectory;
	std::string	filename = imagedirectory.save(darkimage);

	// return a proxy for the image
	return snowstar::getImage(filename, darkimage->pixel_type(), current);
}

/**
 * \brief Return the flat image of the imager
 */
ImagePrx	GuiderI::flatImage(const Ice::Current& current) {
	CallStatistics::count(current);
	// retrieve image
	astro::image::ImagePtr	flatimage = guider->imager().flat();
	if (!flatimage) {
		NotFound	exception;
		exception.cause = std::string("no flat image available");
		throw exception;
	}

	// store image in image directory
	astro::image::ImageDirectory	imagedirectory;
	std::string	filename = imagedirectory.save(flatimage);

	// return a proxy for the image
	return snowstar::getImage(filename, flatimage->pixel_type(), current);
}

/**
 * \brief Whether or not the imager has a dark image
 */
bool    GuiderI::hasDark(const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->imager().hasDark();
}

/**
 * \brief query the use dark
 */
bool	GuiderI::useDark(const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->imager().darksubtract();
}

/**
 * \brief set whether the dark images should be used (if present)
 */
void	GuiderI::setUseDark(bool usedark, const Ice::Current& current) {
	CallStatistics::count(current);
	if (guider->imager().hasDark()) {
		guider->imager().darksubtract(usedark);
		return;
	}
	if (usedark) {
		BadState	exception;
		exception.cause = std::string("have no dark image");
		throw exception;
	}
}

/**
 * \brief start acquisition of a flat image
 */
void    GuiderI::startFlatAcquire(double exposuretime, int imagecount,
				bool useDark,
				const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startFlatAcquire(%.3f, %d) called",
		exposuretime, imagecount);
	try {
		guider->startFlat(exposuretime, imagecount, useDark);
	} catch (const std::exception& x) {
		BadState	exception;
		exception.cause = std::string(x.what());
		throw exception;
	}
}

/**
 * \brief Whether or not the imager has a flat image to apply
 */
bool    GuiderI::hasFlat(const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->imager().hasFlat();
}

/**
 * \brief Whether or not the imager acutally uses the flat image
 */
bool    GuiderI::useFlat(const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->imager().flatdivide();
}

/**
 * \brief Whether or not the imager is supposed to use the flag image
 */
void    GuiderI::setUseFlat(bool useflat, const Ice::Current& current) {
	CallStatistics::count(current);
	if (guider->imager().hasFlat()) {
		guider->imager().flatdivide(useflat);
		return;
	}
	if (useflat) {
		BadState	exception;
		exception.cause = std::string("have not flat image");
		throw exception;
	}
}

/**
 * \brief Whether or not the imager currently uses interpolation
 */
bool    GuiderI::interpolate(const Ice::Current& current) {
	CallStatistics::count(current);
	return guider->imager().interpolate();
}

/**
 * \brief Whether or not the imager currently should use interpolation
 */
void    GuiderI::setInterpolate(bool interpolate, 
			const Ice::Current& current) {
	CallStatistics::count(current);
	if (guider->imager().hasDark()) {
		guider->imager().interpolate(interpolate);
		return;
	}
	if (interpolate) {
		BadState	exception;
		exception.cause = std::string("have not dark image");
		throw exception;
	}
}

/**
 * \brief Register a callback for images taken during the process
 */
void    GuiderI::registerCalibrationImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "register an image callback");
	calibrationimagecallbacks.registerCallback(imagecallback, current);
}

/**
 * \brief Unregister a callback for images
 */
void    GuiderI::unregisterCalibrationImageMonitor(const Ice::Identity& imagecallback,
		const Ice::Current& current) {
	CallStatistics::count(current);
	calibrationimagecallbacks.unregisterCallback(imagecallback, current);
}

/**
 * \brief Update information about the image
 */
void	GuiderI::calibrationImageUpdate(
		const astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "calibrationImageUpdate called");
	calibrationimagecallbacks(data);
}

} // namespace snowstar
