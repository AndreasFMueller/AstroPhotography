/*
 * ImageStreamThread.cpp -- Implementation of the ImageStreamThread class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "ImageStreamThread.h"
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <mutex>
#include <thread>

namespace astro {
namespace camera {

/**
 * \brief Auxiliary function to lauch the thread
 *
 * \param ist	pointer to the ImageStreamThread
 */
static void	imagestreammain(ImageStreamThread *ist) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imagestreammain starting");
	try {
		ist->run();
	} catch (const std::exception& x) {
		std::string msg = stringprintf("thread terminated by %s: %s",
			astro::demangle_string(x).c_str(), x.what());
	} catch (...) {
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imagestreammain terminates");
}

/**
 * \brief Construct a new thread
 *
 * \param stream	ImageStream that is supposed to process the images
 * \param ccd		the CCD that is supposed to retrieve the images
 */
ImageStreamThread::ImageStreamThread(ImageStream& stream, Ccd *ccd)
	: _stream(stream), _ccd(ccd), _running(true),
	  _thread(imagestreammain, this) {
}

/**
 * \brief Kill the thread
 */
ImageStreamThread::~ImageStreamThread() {
	stop();
	_thread.join();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stream thread destroyed");
}

/**
 * \brief main function of the thread
 */
void	ImageStreamThread::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start the image stream thread");
	long	counter = 0;
	try {
		while (_running) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "start new exposure");
			Exposure	exposure = _stream.streamExposure();
			_ccd->startExposure(exposure);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting");
			_ccd->wait();
			ImagePtr	image = _ccd->getImage();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved");

			// create a new entry
			ImageQueueEntry	entry(_ccd->getExposure(), image);
			entry.sequence = counter++;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image entry prepared");

			// hand the entry over to the stream
			_stream(entry);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image added");

			// verify the state
			debug(LOG_DEBUG, DEBUG_LOG, 0, "CCD state: %s",
				CcdState::state2string(_ccd->exposureStatus())
					.c_str());
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error in image loop: %s",
			x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "terminating the image stream thread");
}

/**
 * \brief Stop the thread
 */
void	ImageStreamThread::stop() {
	// make sure no other expsure is started
	_running = false;

	// cancel the current exposure
	try {
		_ccd->cancelExposure();
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("stop cannot cancel "
			"exposure: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		// no need to throw an exception here, because this is 
		// expected to happen in the intervals between new exposures
	}
}

/**
 * \brief wait for the CCD to become idle
 */
void	ImageStreamThread::wait() {
	// XXX wait for the CCD to become idle
}

} // namespace camera
} // namespace astro
