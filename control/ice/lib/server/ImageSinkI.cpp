/*
 * ImageSinkI.cpp -- implementation of the ImageSink callback
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageSinkI.h>
#include <AstroDebug.h>
#include <IceConversions.h>

namespace snowstar {

/**
 * create a new ImageSinkI instance
 */
ImageSinkI::ImageSinkI() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating an ImageSinkI");
}

/**
 * \brief Handle the end of the stream
 */
void	ImageSinkI::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop call received");
}

/**
 * \brief Handle an image from the stream
 */
void	ImageSinkI::image(const ImageQueueEntry& entry,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image sink callback: %s",
		convert(entry.exposure0).toString().c_str());
}

} // namespace snowstar
