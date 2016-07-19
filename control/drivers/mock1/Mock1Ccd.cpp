/*
 * Mock1Ccd.cpp -- implementation of the Mock1 CCD
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <Mock1Ccd.h>
#include <unistd.h>
#include <AstroExceptions.h>

using namespace astro::image;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace mock1 {

/**
 * \brief Start an exposure
 */
void    Mock1Ccd::startExposure(const Exposure& exposure) {
	Ccd::startExposure(exposure);
	frame = exposure.frame();
	state(CcdState::exposing);
	// XXX should start a thread that does nothing for exposuretime seconds
	sleep(1);
	state(CcdState::exposed);
}

/**
 * \brief Query exposure status
 */
CcdState::State Mock1Ccd::exposureStatus() {
	return state();
}

/**
 * \brief Cancel the exposure
 */
void    Mock1Ccd::cancelExposure() {
	if (state() != CcdState::exposing) {
		throw BadState("CCD not exposing");
	}
	state(CcdState::cancelling);
	sleep(1);
	state(CcdState::idle);
}

/**
 * \brief Retrieve the image
 */
ImagePtr    Mock1Ccd::getRawImage() {
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
