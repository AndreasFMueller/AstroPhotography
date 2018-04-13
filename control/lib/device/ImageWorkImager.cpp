/*
 * ImageWorkImager.cpp -- work class for image acquisition
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImager.h>

namespace astro {
namespace camera {

/**
 * \brief Construt an image work object
 */
ImageWorkImager::ImageWorkImager(Imager& imager, const Exposure& exposure)
	: _imager(imager), _exposure(exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"prepare imager based image acquisition on %s, %s",
		_imager.ccd()->name().toString().c_str(),
		_exposure.toString().c_str());
}

/**
 * \brief main method for the image work acquisition thread
 */
void	ImageWorkImager::main(astro::thread::Thread<ImageWorkImager>& /* thread */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image acquisition started");
	try {
		// start exposure
		_imager.startExposure(_exposure);

		// wait for completion
		_imager.wait();

		// retrieve image
		_image = _imager.getImage();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved: %s",
			_image->info().c_str());
	} catch (std::exception& x) {
		std::string	msg = stringprintf("image acquisition "
			"failed: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		// XXX we should really send an reason through the callback
		(*_endcallback)(CallbackDataPtr());
	}
	if (_endcallback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "send image to callback");
		(*_endcallback)(CallbackDataPtr(new ImageCallbackData(_image)));
	}
}

} // namespace camera
} // namespace astro
