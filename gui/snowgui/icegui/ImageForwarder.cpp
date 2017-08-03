/*
 * ImageForwarder.cpp -- implementation of image forwarder
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ImageForwarder.h>
#include <AstroDebug.h>
#include <mutex>

namespace snowgui {

ImageForwarder::ImageForwarder() {
}

void	ImageForwarder::sendImage(astro::image::ImagePtr image,
		std::string title) {
	emit offerImage(image, title);
}

static ImageForwarder	*_forwarder;
static std::once_flag	_forwarder_once;

static void	ImageForwarder_once() {
	_forwarder = new ImageForwarder();
}

ImageForwarder	*ImageForwarder::get() {
	std::call_once(_forwarder_once, ImageForwarder_once);
	return _forwarder;
}

} // namespace snowgui
