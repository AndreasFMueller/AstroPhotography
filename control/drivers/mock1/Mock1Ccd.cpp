/*
 * Mock1Ccd.cpp -- implementation of the Mock1 CCD
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <Mock1Ccd.h>
#include <unistd.h>

using namespace astro::image;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace mock1 {

void    Mock1Ccd::startExposure(const Exposure& exposure) throw (not_implemented) {
	if (state != Exposure::idle) {
		throw std::runtime_error("ccd not idle");
	}
	if (!info.size().bounds(exposure.frame)) {
		throw std::runtime_error("exposure does not fit ccd");
	}
	frame = exposure.frame;
	state = Exposure::exposing;
	sleep(1);
	state = Exposure::exposed;
}

Exposure::State Mock1Ccd::exposureStatus() throw (not_implemented) {
	return state;
}

void    Mock1Ccd::cancelExposure() throw (not_implemented) {
	if (state != Exposure::idle) {
		throw std::runtime_error("ccd not idle");
	}
	state = Exposure::cancelling;
	sleep(1);
	state = Exposure::idle;
}

ImagePtr    Mock1Ccd::getImage() throw (not_implemented) {
	Image<unsigned char>	image(info.size());
	image.fill(128);
	ImageSize	blocksize(5, 5);
	for (int i = 0; i <= cameraid; i++) {
		ImageRectangle	rect(ImagePoint(10 * i, 10), blocksize);
		image.fill(rect, i);
	}
	for (int i = 0; i <= ccd; i++) {
		ImageRectangle	rect(ImagePoint(10 * i, 20), blocksize);
		image.fill(rect, 255 - i);
	}
	Image<unsigned char>	*result = new Image<unsigned char>(image, frame);
	return ImagePtr(result);
}

} // namespace mock1
} // namespace camera
} // namespace astro
