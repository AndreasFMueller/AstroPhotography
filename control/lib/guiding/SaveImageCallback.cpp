/*
 * SaveImageCallback.cpp -- callback that saves an image in an image directory
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCallback.h>
#include <AstroGuiding.h>
#include <AstroDebug.h>

using namespace astro::guiding;

namespace astro {
namespace callback {

/**
 * \brief Save an image to the image directory
 *
 * \param data	callback data of type ImageCallbackData
 * \return the same callback data
 */
CallbackDataPtr	SaveImageCallback::operator()(CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image callback called");
	image::ImagePtr	image;
	
	// first find out whether this is image callback data
	ImageCallbackData       *icb
		= dynamic_cast<ImageCallbackData *>(&*data);
	if (NULL != icb) {
		image = icb->image();
	}

	// if we have no image, we cannot handle the callback
	if (!image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no image found");
		return data;
	}

	// add the image to the directory
	image::ImageDirectory	id;
	id.save(image);

	// return empty callback ptr
	return data;
}

} // namespace callback
} // namespace astro
