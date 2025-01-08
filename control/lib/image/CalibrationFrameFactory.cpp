/**
 * CalibrationFrameFactory.cpp -- compute calibration frames
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroCalibration.h>
#include <AstroIO.h>
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
 *
 * \param images	the image sequence to create the image from
 */
ImagePtr	CalibrationFrameFactory::operator()(
			const ImageSequence& images) const {
	std::string	msg = astro::stringprintf("base class factory method "
		"called (%d images), probably an error", images.size());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

// The following macro simplifies the code on the copyMeta method
#define	copy_metadata(name)						\
	if (firstimage->hasMetadata(name)) {				\
		calframe->setMetadata(firstimage->getMetadata(name));	\
	}

/**
 * \brief Copy metadata to the calibration frame created by the factory
 *
 * \param calframe	the calibration frame to copy metadata to
 * \param images	the image sequence from which to get the
 *			common metadata
 */
void	CalibrationFrameFactory::copyMetadata(ImagePtr calframe,
		const ImageSequence& images,
		const std::string& purpose) const {
	// if there aren't any images, skip ahead to the new meta data
	if (images.size() > 0) {
		// get the first image, which is used to get the common 
		// metadata from
		ImagePtr firstimage = *images.begin();

		// copy information about the images
		copy_metadata("EXPTIME");
		copy_metadata("XBINNING");
		copy_metadata("YBINNING");
		copy_metadata("SET-TEMP");
		copy_metadata("CCD-TEMP");
		copy_metadata("DATE-OBS");
		calframe->setMosaicType(firstimage->getMosaicType());

		// copy information about the project
		copy_metadata("CAMERA");
		copy_metadata("INSTRUME");
		copy_metadata("PROJECT");
	}

	// add common information about subframes
	calframe->setMetadata(io::FITSKeywords::meta("PURPOSE", purpose));
	calframe->setMetadata(io::FITSKeywords::meta("CALSUBFM",
		(long)images.size()));
}

} // calibration
} // astro
