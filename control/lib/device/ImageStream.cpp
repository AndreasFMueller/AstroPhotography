/*
 * ImageStream.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

ImageStream::ImageStream(unsigned long _maxqueuelength)
	: ImageQueue(_maxqueuelength) {
	private_data = NULL;
}

ImageStream::~ImageStream() {
}

void	ImageStream::startStream(const Exposure& /* exposure */) {
	throw CannotStream();
}

void	ImageStream::stopStream() {
	throw CannotStream();
}

void	ImageStream::streamExposure(const Exposure& exposure) {
	_streamexposure = exposure;
}

void	ImageStream::operator()(const ImageQueueEntry& entry) {
	if (_imagesink) {
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

} // namespace camera
} // namespace astro
