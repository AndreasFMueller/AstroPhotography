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

DeviceName	ccdname(::AtikCamera *camera, const std::string& name) {
	DeviceName	cn = cameraname(camera);
	return DeviceName(cn, DeviceName::Ccd, name);
}

DeviceName	filterwheelname(::AtikCamera *camera) {
	unsigned int serial = camera->getSerialNumber();
	return DeviceName(DeviceName::Filterwheel, "atik",
		astro::stringprintf("%u", serial));
}

DeviceName	guiderportname(::AtikCamera *camera) {
	unsigned int serial = camera->getSerialNumber();
	return DeviceName(DeviceName::Guiderport, "atik",
		astro::stringprintf("%u", serial));
}

DeviceName	coolername(::AtikCamera *camera) {
	unsigned int serial = camera->getSerialNumber();
	return DeviceName(DeviceName::Cooler, "atik",
		astro::stringprintf("%u", serial));
}

} // namespace atik
} // namespace camera
} // namespace astro
