/**
 * CalibrationFrameFactory.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroCalibration.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace calibration {

/**
 * \brief Factory method
 *
 * This is the factory method, it takes an image sequence and produces
 * a calibration image. The base class of course has no data on which
 * to base the creation of a calibration image, so it just returns an
 * empty image pointer.
 */
ImagePtr	CalibrationFrameFactory::operator()(const ImageSequence&) const {
	debug(LOG_ERR, DEBUG_LOG, 0, "base class factory method called, "
		"probably an error");
	return ImagePtr();
}

void	CalibrationFrameFactory::copyMetadata(ImagePtr calframe,
		ImagePtr firstimage) const {

	// copy information about the images
	if (firstimage->hasMetadata("EXPTIME")) {
		calframe->setMetadata(firstimage->getMetadata("EXPTIME"));
	}
	if (firstimage->hasMetadata("XBINNING")) {
		calframe->setMetadata(firstimage->getMetadata("XBINNING"));
	}
	if (firstimage->hasMetadata("YBINNING")) {
		calframe->setMetadata(firstimage->getMetadata("YBINNING"));
	}
	if (firstimage->hasMetadata("SET-TEMP")) {
		calframe->setMetadata(firstimage->getMetadata("SET-TEMP"));
	}
	if (firstimage->hasMetadata("CCD-TEMP")) {
		calframe->setMetadata(firstimage->getMetadata("CCD-TEMP"));
	}
	if (firstimage->hasMetadata("DATE-OBS")) {
		calframe->setMetadata(firstimage->getMetadata("DATE-OBS"));
	}

	// copy information about the project
	if (firstimage->hasMetadata("CAMERA")) {
		calframe->setMetadata(firstimage->getMetadata("CAMERA"));
	}
	if (firstimage->hasMetadata("INSTRUME")) {
		calframe->setMetadata(firstimage->getMetadata("INSTRUME"));
	}
	if (firstimage->hasMetadata("PROJECT")) {
		calframe->setMetadata(firstimage->getMetadata("PROJECT"));
	}
	calframe->setMosaicType(firstimage->getMosaicType());
}

} // calibration
} // astro
