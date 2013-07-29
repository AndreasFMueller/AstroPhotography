/*
 * UvcCcd.cpp -- CCD implementation for UVC cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UvcCcd.h>
#include <AstroDebug.h>
#include <UvcUtils.h>
#include <AstroFilter.h>
#include <AstroOperators.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::image::operators;

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

UvcCcdYUY2::UvcCcdYUY2(const CcdInfo& info, int interface, int format,
	int frame, UvcCamera& camera)
		: UvcCcd(info, interface, format, frame, camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating YUY2 CCD");
}

UvcCcdY800::UvcCcdY800(const CcdInfo& info, int interface, int format,
	int frame, UvcCamera& camera)
		: UvcCcd(info, interface, format, frame, camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating Y800 CCD");
}

UvcCcdBY8::UvcCcdBY8(const CcdInfo& info, int interface, int format,
	int frame, UvcCamera& camera)
		: UvcCcd(info, interface, format, frame, camera) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating BY8 CCD");
}

/**
 * \brief Start an Exposure on an UVC camera
 *
 * \param exposure 	Exposure parameters
 */
void	UvcCcd::startExposure(const Exposure& exposure) throw(not_implemented) {
	this->exposure = exposure;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");
	if (exposure.frame.size() != info.size) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot take subimages");
		throw UvcError("UVC driver cannot take subimages");
	}

	if ((exposure.frame.origin().x() != 0) || (exposure.frame.origin().y() != 0)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "UVC images cannot have offset");
		throw UvcError("UVC driver cannot have offsets");
	}

	// select interface, format and frame
	camera.selectFormatAndFrame(interface, format, frame);

	// should also disable automatic white balance
	//camera.disableAutoWhiteBalance();

	// set exposure time
	camera.setExposureTime(exposure.exposuretime);

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

		// convert the frame, this depends on the frame type
		ImagePtr	imageptr = frameToImage(*frameptr);

		// add the metadata
		addMetadata(*imageptr);

		// add image to result set
		result.push_back(imageptr);
	}

	// set state back to not done
	state = Exposure::idle;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning sequence with %d images",
		result.size());
	return result;
}

/**
 * \brief Convert an YUYV frame to an image with YUYV pixels
 *
 * \param frame	frame from USB transfer
 */
ImagePtr	UvcCcdYUY2::frameToImage(const Frame& frame) const {
	ImageSize	size(frame.getWidth(), frame.getHeight());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "building YUY2 image %u x %u",
		size.width(), size.height());
	Image<YUYV<unsigned char> >	*image
		= new Image<YUYV<unsigned char> >(size);
	for (unsigned int i = 0; i < size.getPixels(); i++) {
		image->pixels[i].y = frame[2 * i];
		image->pixels[i].uv = frame[2 * i + 1];
	}
	FlipOperator<YUYV<unsigned char> >	flip;
	flip(*image);
	return ImagePtr(image);
}

/**
 * \brief Convert luminance only image
 *
 * Images built from formats with guid Y800 are luminance only images,
 * the produce an image with only a single plane
 */
ImagePtr	UvcCcdY800::frameToImage(const Frame& frame) const {
	ImageSize	size(frame.getWidth(), frame.getHeight());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "building Y800 image %u x %u",
		size.width(), size.height());
	Image<unsigned char>	*image = new Image<unsigned char>(size);
	for (unsigned int i = 0; i < size.getPixels(); i++) {
		image->pixels[i] = frame[i];
	}
	FlipOperator<unsigned char>	flip;
	flip(*image);
	return ImagePtr(image);
}

/**
 * \brief Convert a Bayer mosaic image to an Image object
 *
 * \param frame	
 */
ImagePtr	UvcCcdBY8::frameToImage(const Frame& frame) const {
	ImageSize	size(frame.getWidth(), frame.getHeight());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "building BY8 image %u x %u",
		size.width(), size.height());
	Image<unsigned char>	*image = new Image<unsigned char>(size);
	image->setMosaicType(ImageBase::BAYER_RGGB);
	for (unsigned int i = 0; i < size.getPixels(); i++) {
		image->pixels[i] = frame[i];
	}
	FlipOperator<unsigned char>	flip;
	flip(*image);
	return ImagePtr(image);
}

} // namespace uvc
} // namespace camera
} // namespace astro

