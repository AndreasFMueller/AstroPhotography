/*
 * ImageStream.cpp -- Implementation of the ImageStream class
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include "ImageStreamThread.h"

namespace astro {
namespace camera {

/**
 * \brief Construct a stream
 */
ImageStream::ImageStream(unsigned long _maxqueuelength)
	: ImageQueue(_maxqueuelength) {
	private_data = NULL;
}

/**
 * \brief Destroy the stream
 *
 * If it has a running thread, we have to kill it
 */
ImageStream::~ImageStream() {
	if (private_data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelling thread");
		cleanup();
	}
}

void	ImageStream::cleanup() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	ImageStreamThread	*t = (ImageStreamThread *)private_data;
	try {
		t->stop();
	} catch (...) {
	}
	delete t;
	private_data = NULL;
}

/**
 * \brief start a stream with a given exposure structure
 */
void	ImageStream::startStream(const Exposure& exposure) {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_streamexposure = exposure;
	// make sure the stream is not running yet
	if (private_data) {
		ImageStreamThread	*t = (ImageStreamThread *)private_data;
		if (t->running()) {
			throw std::logic_error("stream already running");
		}
		cleanup();
	}

	// find out whether we also are a CCD, in which case we can really
	// start the thread
	Ccd	*ccd = dynamic_cast<Ccd *>(this);
	if (NULL == ccd) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a CCD, cannot stream");
		throw CannotStream();
	}

	// start the thread with the information we have gathered
	ImageStreamThread	*t = new ImageStreamThread(*this, ccd);
	private_data = t;
}

/**
 * \brief Stop the stream
 */
void	ImageStream::stopStream() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (private_data) {
		cleanup();
	}
}

/**
 * \brief change the stream exposure
 */
void	ImageStream::streamExposure(const Exposure& exposure) {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	_streamexposure = exposure;
}

/**
 * \brief get the current exposure settings
 */
Exposure	ImageStream::streamExposure() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return _streamexposure;
}

/**
 * \brief Find out whether stream is still streaming
 */
bool	ImageStream::streaming() {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (NULL == private_data) {
		return false;
	}
	ImageStreamThread	*t = (ImageStreamThread *)private_data;
	return t->running();
}

/**
 * \brief Process an image entry
 *
 * This method sends the entry to the queue if no sink is defined, but
 * if there is a sink, the image is sent there.
 */
void	ImageStream::operator()(const ImageQueueEntry& entry) {
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new queue entry received");
	// check whether streaming is already turned off, in which case
	// we should not process any images (we shouldn't even be called ;-)
	if (!streaming()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %s sent after stop, dropped",
			entry.exposure.toString().c_str());
		return;
	}
	if (_imagesink) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending entry to sink");
		(*_imagesink)(entry);
	} else {
		ImageQueueEntry	newentry(entry);
		try {
			ImageQueue::add(newentry);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new queue entry %ld",
				newentry.sequence);
		} catch (ImageDropped& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "entry dropped");
		}
	}
}

/**
 * \brief Registering an image sink
 */
void	ImageStream::imagesink(ImageSink *i) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registered sink %s",
		demangle(typeid(*i).name()).c_str());
	_imagesink = i;
}

} // namespace camera
} // namespace astro
