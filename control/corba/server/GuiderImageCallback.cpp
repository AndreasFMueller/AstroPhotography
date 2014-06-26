/*
 * GuiderImageCallback.cpp -- Callback that sends images to the monitor
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <GuiderImageCallback.h>
#include <Guider_impl.h>
#include <AstroCallback.h>
#include <Conversions.h>

namespace Astro {

/**
 * \brief Create the callback
 *
 * We assume that this is equivalent to createing a new guider run
 */
GuiderImageCallback::GuiderImageCallback(Guider_impl& guider)
	: _guider(guider) {
}

/**
 * \brief process a new image
 *
 * \param data	a callback data pointer
 */
astro::callback::CallbackDataPtr GuiderImageCallback::operator()(
	astro::callback::CallbackDataPtr data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image received");

	// get the image data from the callback argument
	astro::callback::ImageCallbackData	*image
		= dynamic_cast<astro::callback::ImageCallbackData *>(&*data);
	if (NULL == image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not image data");
		return data;
	}
	Astro::TrackingImage_var	trackingimage
		= new Astro::TrackingImage();
	trackingimage->size = astro::convert(image->image()->size());
	int	width = trackingimage->size.width;
	int	height = trackingimage->size.height;

	// the access to the pixel array
	astro::image::Image<unsigned short>	*im
		= dynamic_cast<astro::image::Image<unsigned short> *>(
			&*image->image());
	if (NULL == im) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"only short images can be monitored");
		return data;
	}

	// prepare the imagedata
	trackingimage->imagedata.length(width * height);

	// get the pixels
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			trackingimage->imagedata[x + width * y] = im->pixel(x, y);
		}
	}

	// update all image monitors
	_guider.update(trackingimage);

	return data;
}

} // namespace Astro
