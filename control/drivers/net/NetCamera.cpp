/*
 * NetCamera.cpp -- Network based camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetCamera.h>
#include <NetCcd.h>

using namespace astro::image;

namespace astro {
namespace camera {
namespace net {

NetCamera::NetCamera(Astro::Camera_var camera) : _camera(camera) {
	// retrieve Ccds from the camera reference and fill the CCDinfo
}

CcdPtr	NetCamera::getCcd0(size_t ccdid) {
	return CcdPtr();
}

FilterWheelPtr	NetCamera::getFilterWheel0() {
	return FilterWheelPtr();
}

GuiderPortPtr	NetCamera::getGuiderPort0() {
	return GuiderPort();
}

} // namespace net
} // namespace camera
} // namespace astro

