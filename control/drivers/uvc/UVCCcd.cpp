/*
 * UVCCcd.cpp 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UVCCcd.h>
#include <debug.h>
#include <UVCUtils.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace uvc {

UVCCcd::UVCCcd(const CcdInfo& info, int _interface, int _format, int _frame,
	UVCCamera& _camera)
	: Ccd(info), interface(_interface), format(_format), frame(_frame),
	  camera(_camera) {
}

void	UVCCcd::startExposure(const Exposure& exposure) throw(not_implemented) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting exposure");
	if (exposure.frame.size != info.size) {
		throw UVCError("UVC driver cannot take subimages");
	}

	if ((exposure.frame.origin.x != 0) || (exposure.frame.origin.y != 0)) {
		throw UVCError("UVC driver cannot have offsets");
	}

	// select interface, format and frame
	camera.selectFormatAndFrame(interface, format, frame);

	// set exposure time
	camera.setExposureTime(exposure.exposuretime);

	// XXX should also disable automatic white balance
}

ImagePtr	UVCCcd::getImage() throw(not_implemented) {
	throw not_implemented("get image not yet implemented");
}

} // namespace uvc
} // namespace camera
} // namespace astro

