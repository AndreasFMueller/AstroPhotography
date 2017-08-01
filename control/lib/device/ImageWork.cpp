/*
 * ImageWork.cpp -- work class for image acquisition
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImager.h>

namespace astro {
namespace camera {

/**
 * \brief Construt an image work object
 */
ImageWork::ImageWork(CcdPtr ccd, const Exposure& exposure)
	: _ccd(ccd), _exposure(exposure) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "prepare image acquisition on %s, %s",
		_ccd->name().toString().c_str(), _exposure.toString().c_str());
}

/**
 * \brief main method for the image work acquisition thread
 */
void	ImageWork::main(astro::thread::Thread<ImageWork>& /* thread */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image acquisition started");
	try {
		// start exposure
		_ccd->startExposure(_exposure);

		// wait for completion
		_ccd->wait();

		// retrieve image
		_image = _ccd->getImage();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved");
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "image acquisition failed");
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
