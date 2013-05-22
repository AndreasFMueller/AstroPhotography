/*
 * UvcCcd.cpp -- CCD implementation for UVC cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UvcCcd.h>
#include <debug.h>
#include <UvcUtils.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace uvc {

/**
 * \brief Construct a UvcCcd
 *
 * \param info
 * \param _interface
 * \param _format
 * \param _frame
 * \param _camera
 */
UvcCcd::UvcCcd(const CcdInfo& info, int _interface, int _format, int _frame,
	UvcCamera& _camera)
	: Ccd(info), interface(_interface), format(_format), frame(_frame),
	  camera(_camera) {
}

/**
 * \brief Start an Exposure on an UVC camera
 *
 * \param exposure 	Exposure parameters
 */
void	UvcCcd::startExposure(const Exposure& exposure) throw(not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");
	if (exposure.frame.size != info.size) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot take subimages");
		throw UvcError("UVC driver cannot take subimages");
	}

	if ((exposure.frame.origin.x != 0) || (exposure.frame.origin.y != 0)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "UVC images cannot have offset");
		throw UvcError("UVC driver cannot have offsets");
	}

	// select interface, format and frame
	camera.selectFormatAndFrame(interface, format, frame);

	// set exposure time
	camera.setExposureTime(exposure.exposuretime);

	// XXX should also disable automatic white balance

	// status
	state = Exposure::exposed;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure started");
}

/**
 * \brief Get a single image
 *
 * A UVC camera can more easily retrieve an image sequence than an 
 * individual image. So we just retrieve an image of one image, and
 * extract the image from the sequence.
 */
ImagePtr	UvcCcd::getImage() throw(not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get an image");
	// retrieve an image
	ImageSequence	sequence = getImageSequence(1);
	return *sequence.begin();
}

/**
 * \brief Get an image sequence
 *
 * Get an image sequence
 * \param imagecount	length of the image sequence
 */
ImageSequence	UvcCcd::getImageSequence(unsigned int imagecount)
	throw(not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get an image sequence of %d images",
		imagecount);
	ImageSequence	result;

	// retrieve a sequence of frames
	std::vector<FramePtr>	frames = camera.getFrames(interface,
		imagecount);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d frames", frames.size());
	
	// now convert every frame into an image
	std::vector<FramePtr>::iterator	i;
	for (i = frames.begin(); i != frames.end(); i++) {
		FramePtr	frameptr = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image has size %d x %d",
			frameptr->getWidth(), frameptr->getHeight());

		// XXX convert the frame, this depends on the frame type
	}

	// set state back to not done
	state = Exposure::idle;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning sequence with %d images",
		result.size());
	return result;
}

} // namespace uvc
} // namespace camera
} // namespace astro

