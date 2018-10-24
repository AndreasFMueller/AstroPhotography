/*
 * TakeImageSink.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <TakeImageSink.h>

namespace snowgui {

/**
 * \brief Create the image sink
 */
TakeImageSink::TakeImageSink(QObject *parent)
	: QObject(parent), snowstar::ImageSink() {
	qRegisterMetaType<astro::image::ImagePtr>("astro::image::ImagePtr");

	_enabled = true;
}

/**
 * \brief Destroy the image sink
 */
TakeImageSink::~TakeImageSink() {
}

/**
 * \brief handle a new image
 */
void	TakeImageSink::image(const snowstar::ImageQueueEntry& entry,
		const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got image of size %dx%d, size = %ld",
		entry.exposure0.frame.size.width,
		entry.exposure0.frame.size.height,
		entry.imagedata.size());
	if (!_enabled) {
		return;
	}

	// convert the image to an astro::image::ImagePtr
	astro::image::ImagePtr  image = snowstar::convertfile(entry.imagedata);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"image has depth %d, bits_per_pixel = %d",
		image->planes(), image->bitsPerPixel());

        // send the image to the preview
	emit newImage(image);
}

/**
 * \brief Handle the end of the stream
 */
void	TakeImageSink::stop(const Ice::Current& /* current */) {
	emit finished();
}

/**
 *  \brief Slot to turn the sink on/off
 */
void	TakeImageSink::setEnabled(bool e) {
	_enabled = e;
}

} // namespace snowgui
