/*
 * AsiStream.cpp -- implementation of the AsiStream
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiStream.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief main function for asi stream thread
 */
static void asistreammain(AsiStream* asistream) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start thread run");
	asistream->run();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "thread completed");
}

/**
 * \brief Construct a new stream thread
 */
AsiStream::AsiStream(AsiCcd *ccd) : _ccd(ccd), _running(true),
	_thread(asistreammain, this) {
}

/**
 * \brief Destructor for th thread
 */
AsiStream::~AsiStream() {
	stop();
	_thread.join();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stream terminated");
}

/**
 * \brief Stop the thread
 */
void	AsiStream::stop() {
	_running = false;
}

/**
 * \brief Main function of the stream thread
 */
void	AsiStream::run() {
	Exposure	exposure = _ccd->ImageStream::streamExposure();
	while (_running) {
		// has the exposure changed?
		if (exposure != _ccd->ImageStream::streamExposure()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"modified exposure settings: %s",
				exposure.toString().c_str());
			exposure = _ccd->ImageStream::streamExposure();
			_ccd->setExposure(exposure);
		}

		// 

		// get image
		ImagePtr	image = _ccd->getRawImage();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new %s image",
			image->size().toString().c_str());

		// send image to stream
		_ccd->ImageStream::add(_ccd->ImageStream::streamExposure(),
			image);
	}
}

} // namespace asi
} // namespace camera
} // namespace astro
