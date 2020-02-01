/*
 * GuiderIImage.cpp -- imaging related methods for the guider
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
 * \brief start imaging with a given exposure
 */
void	GuiderI::startImaging(const Exposure& exposure,
                                const Ice::Current& current) {
	CallStatistics::count(current);
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
	CallStatistics::count(current);
	// retrieve image
	astro::image::ImagePtr	image = guider->getImaging();
	if (!image) {
		throw NotFound("no image available");
	}

	// store image in image directory
	astro::image::ImageDirectory	imagedirectory;
	std::string	filename = imagedirectory.save(image);

	// return a proxy for the image
	return snowstar::getImage(filename, image->pixel_type(), current);
}

} // namespace snowstar
