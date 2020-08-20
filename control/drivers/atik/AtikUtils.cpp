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

DeviceName	cameraname(AtikCamera& camera) {
	unsigned int serial = camera.getSerialNumber();
	return DeviceName(DeviceName::Camera, "atik",
		astro::stringprintf("%u", serial));
}


DeviceName	ccdname(AtikCamera& camera, const std::string& name) {
	DeviceName	cn = cameraname(camera);
	return DeviceName(cn, DeviceName::Ccd, name);
}

DeviceName	ccdname(::AtikCamera *camera, const std::string& name) {
	DeviceName	cn = cameraname(camera);
	return DeviceName(cn, DeviceName::Ccd, name);
}


DeviceName	filterwheelname(AtikCamera& camera) {
	return DeviceName(camera.name(), DeviceName::Filterwheel);
}

DeviceName	filterwheelname(::AtikCamera *camera) {
	return DeviceName(cameraname(camera), DeviceName::Filterwheel);
}


DeviceName	guideportname(AtikCamera& camera) {
	return DeviceName(camera.name(), DeviceName::Guideport);
}

DeviceName	guideportname(::AtikCamera *camera) {
	return DeviceName(cameraname(camera), DeviceName::Guideport);
}

DeviceName	coolername(AtikCamera& camera) {
	return DeviceName(camera.name(), DeviceName::Cooler);
}

DeviceName	coolername(::AtikCamera *camera) {
	return DeviceName(cameraname(camera), DeviceName::Cooler);
}

} // namespace atik
} // namespace camera
} // namespace astro
