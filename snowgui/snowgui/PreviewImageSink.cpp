/*
 * PreviewImageSink.cpp --  implementation of the preview image sink
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <PreviewImageSink.h>
#include <AstroDebug.h>
#include <IceConversions.h>
#include "Image2Pixmap.h"

PreviewImageSink::PreviewImageSink(PreviewWindow *preview)
	: _preview(preview) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview image sink created");
}

PreviewImageSink::~PreviewImageSink() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview image sink destroyed");
}

void	PreviewImageSink::image(const snowstar::ImageQueueEntry& entry,
			const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image of size %dx%d, size = %ld",
		entry.exposure0.frame.size.width,
		entry.exposure0.frame.size.height,
		entry.imagedata.size());
	// convert the image to an astro::image::ImagePtr
	astro::image::ImagePtr	image = snowstar::convertfile(entry.imagedata);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"image has depth %d, bits_per_pixel = %d",
		image->planes(), image->bitsPerPixel());

	// send the image to the preview
	_preview->setImage(image);
}

void	PreviewImageSink::stop(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop called");
}
