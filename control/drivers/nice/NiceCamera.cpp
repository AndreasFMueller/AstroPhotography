/*
 * NiceCamera.cpp -- ICE camera wrapper
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceCamera.h>
#include <NiceCcd.h>
#include <NiceGuidePort.h>
#include <NiceFilterWheel.h>
#include <IceConversions.h>

namespace astro {
namespace camera {
namespace nice {

NiceCamera::NiceCamera(snowstar::CameraPrx camera, const DeviceName& devicename)
	: astro::camera::Camera(devicename), NiceDevice(devicename),
	  _camera(camera) {
	// get Ccd information
	int	n = _camera->nCcds();
	for (int i = 0; i < n; i++) {
		ccdinfo.push_back(convert(_camera->getCcdinfo(i)));
	}
}

NiceCamera::~NiceCamera() {
}

CcdPtr	NiceCamera::getCcd0(size_t id) {
	snowstar::CcdPrx	ccd = _camera->getCcd(id);
	std::string	servicename = name()[1];
	astro::device::nice::DeviceNicer	nicer(servicename);
	return CcdPtr(new NiceCcd(ccd, nicer(ccd->getName())));
}

bool	NiceCamera::hasFilterWheel() const {
	return _camera->hasFilterWheel();
}

FilterWheelPtr	NiceCamera::getFilterWheel0() {
	snowstar::FilterWheelPrx	filterwheel
		= _camera->getFilterWheel();
	return FilterWheelPtr(new NiceFilterWheel(filterwheel,
		filterwheel->getName()));
}

bool	NiceCamera::hasGuidePort() const {
	return _camera->hasGuidePort();
}

GuidePortPtr	NiceCamera::getGuidePort0() {
	snowstar::GuidePortPrx	guideport = _camera->getGuidePort();
	return GuidePortPtr(new NiceGuidePort(guideport,
		nice(guideport->getName())));
}

} // namespace nice
} // namespace camera
} // namespace astro
