/*
 * AtikUtils.cpp -- ATIK utilities implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AtikUtils.h>
#include <AstroFormat.h>

namespace astro {
namespace camera {
namespace atik {

DeviceName	cameraname(::AtikCamera *camera) {
	unsigned int serial = camera->getSerialNumber();
	return DeviceName(DeviceName::Camera, "atik",
		astro::stringprintf("%u", serial));
}

} // namespace atik
} // namespace camera
} // namespace astro
