/*
 * NiceCamera.cpp -- ICE camera wrapper
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceCamera.h>
#include <NiceCcd.h>
#include <NiceGuiderPort.h>
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
	return CcdPtr(new NiceCcd(ccd, ccd->getName()));
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

bool	NiceCamera::hasGuiderPort() const {
	return _camera->hasGuiderPort();
}

GuiderPortPtr	NiceCamera::getGuiderPort0() {
	snowstar::GuiderPortPrx	guiderport = _camera->getGuiderPort();
	return GuiderPortPtr(new NiceGuiderPort(guiderport,
		nice(guiderport->getName())));
}

} // namespace nice
} // namespace camera
} // namespace astro
