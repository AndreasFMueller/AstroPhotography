/*
 * DeviceLocator.cpp -- Device locator base class
 *
 * (c) 2013 Prof Dr Andreas Mueller,
 */
#include <AstroLocator.h>
#include <AstroCamera.h>
#include <config.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>

using namespace astro::camera;
using namespace astro::device;

namespace astro {
namespace device {

//////////////////////////////////////////////////////////////////////
// DeviceCacheAdapter implementation
//////////////////////////////////////////////////////////////////////

template<>
AdaptiveOpticsPtr	DeviceCacheAdapter<AdaptiveOptics>::get0(const DeviceName& name) {
	return _locator->getAdaptiveOptics0(name);
}

template<>
CameraPtr	DeviceCacheAdapter<Camera>::get0(const DeviceName& name) {
	return _locator->getCamera0(name);
}

template<>
CcdPtr	DeviceCacheAdapter<Ccd>::get0(const DeviceName& name) {
	return _locator->getCcd0(name);
}

template<>
CoolerPtr	DeviceCacheAdapter<Cooler>::get0(const DeviceName& name) {
	return _locator->getCooler0(name);
}

template<>
FilterWheelPtr	DeviceCacheAdapter<FilterWheel>::get0(const DeviceName& name) {
	return _locator->getFilterWheel0(name);
}

template<>
FocuserPtr	DeviceCacheAdapter<Focuser>::get0(const DeviceName& name) {
	return _locator->getFocuser0(name);
}

template<>
GuiderPortPtr	DeviceCacheAdapter<GuiderPort>::get0(const DeviceName& name) {
	return _locator->getGuiderPort0(name);
}

template<>
MountPtr	DeviceCacheAdapter<Mount>::get0(const DeviceName& name) {
	return _locator->getMount0(name);
}

//////////////////////////////////////////////////////////////////////
// DeviceLocator implementation
//////////////////////////////////////////////////////////////////////
DeviceLocator::DeviceLocator() :
	aocache(this), cameracache(this), ccdcache(this), coolercache(this),
	filterwheelcache(this), focusercache(this), guiderportcache(this),
	mountcache(this) {
}

DeviceLocator::~DeviceLocator() {
}

std::string	DeviceLocator::getName() const {
	return std::string("generic device");
}

std::string	DeviceLocator::getVersion() const {
	return std::string(VERSION);
}

std::vector<std::string>	DeviceLocator::getDevicelist(
					const DeviceName::device_type device) {
	std::vector<std::string>	devices;
	return devices;
}

std::vector<DeviceName>	DeviceLocator::getDeviceList(
					const DeviceName::device_type device) {
	std::vector<DeviceName>	devices;
	std::vector<std::string>	l = getDevicelist(device);
	std::vector<std::string>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		devices.push_back(DeviceName(*i));
	}
	return devices;
}

astro::camera::AdaptiveOpticsPtr	DeviceLocator::getAdaptiveOptics0(const DeviceName& name) {
	throw std::runtime_error("adaptive optics not implemented");
}

astro::camera::CameraPtr	DeviceLocator::getCamera0(const DeviceName& name) {
	throw std::runtime_error("cameras not implemented");
}

astro::camera::CcdPtr	DeviceLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find ccd %s", name.toString().c_str());
	DeviceName	cameraname = name.parent(DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);
	int	nccds = camera->nCcds();
	for (int i = 0; i < nccds; i++) {
		CcdInfo	info = camera->getCcdInfo(i);
		if (info.name() == name) {
			return camera->getCcd(i);
		}
	}
	throw NotFound(stringprintf("ccd %s not found",
		name.toString().c_str()));
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find guiderport %s",
		name.toString().c_str());
	DeviceName	cameraname = name.parent(DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);
	if (camera->hasGuiderPort()) {
		return camera->getGuiderPort();
	}
	throw NotFound(stringprintf("guiderport %s not found",
		name.toString().c_str()));
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel0(const DeviceName& name) {
	throw std::runtime_error("filterwheel not implemented");
}

astro::camera::CoolerPtr	DeviceLocator::getCooler0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find cooler %s",
		name.toString().c_str());
	DeviceName	ccdname = name.parent(DeviceName::Ccd);
	CcdPtr	ccd = this->getCcd(ccdname);
	if (ccd->hasCooler()) {
		return ccd->getCooler();
	}
	throw NotFound(stringprintf("cooler %s not found",
		name.toString().c_str()));
}

astro::camera::FocuserPtr	DeviceLocator::getFocuser0(const DeviceName& name) {
	throw std::runtime_error("focuser not implemented");
}

astro::device::MountPtr	DeviceLocator::getMount0(const DeviceName& name) {
	throw std::runtime_error("mount not implemented");
}

astro::camera::AdaptiveOpticsPtr	DeviceLocator::getAdaptiveOptics(const std::string& name) {
	return aocache.get(name);
}

astro::camera::CameraPtr	DeviceLocator::getCamera(const std::string& name) {
	return cameracache.get(name);
}

astro::camera::CcdPtr	DeviceLocator::getCcd(const std::string& name) {
	return ccdcache.get(name);
}

astro::camera::CoolerPtr	DeviceLocator::getCooler(const std::string& name) {
	return coolercache.get(name);
}

astro::camera::CameraPtr	DeviceLocator::getCamera(size_t index) {
	std::vector<std::string>	cameras = this->getDevicelist();
	if (index >= cameras.size()) {
		throw std::runtime_error("cannot create a camera from an index");
	}
	return getCamera(cameras[index]);
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel(const std::string& name) {
	return filterwheelcache.get(name);
}

astro::camera::FocuserPtr	DeviceLocator::getFocuser(const std::string& name) {
	return focusercache.get(name);
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort(const std::string& name) {
	return guiderportcache.get(name);
}

astro::device::MountPtr	DeviceLocator::getMount(const std::string& name) {
	return mountcache.get(name);
}

//////////////////////////////////////////////////////////////////////
// Locator Adapter class
//////////////////////////////////////////////////////////////////////

// Adaptive Optics
template<>
astro::camera::AdaptiveOpticsPtr	LocatorAdapter<astro::camera::AdaptiveOptics>::get(const DeviceName& name) {
	return _locator->getAdaptiveOptics(name);
}

template<>
astro::camera::AdaptiveOpticsPtr	LocatorAdapter<astro::camera::AdaptiveOptics>::get0(const DeviceName& name) {
	return _locator->getAdaptiveOptics(name);
}

// Camera
template<>
astro::camera::CameraPtr	LocatorAdapter<astro::camera::Camera>::get(
					const DeviceName& name) {
        return _locator->getCamera(name);
}

template<>
astro::camera::CameraPtr	LocatorAdapter<astro::camera::Camera>::get0(
					const DeviceName& name) {
        return _locator->getCamera(name);
}

// Ccd
template<>
astro::camera::CcdPtr	LocatorAdapter<astro::camera::Ccd>::get(
					const DeviceName& ccdname) {
	return getCameraChild(ccdname);
}

template<>
astro::camera::CcdPtr	LocatorAdapter<astro::camera::Ccd>::get0(
					const DeviceName& ccdname) {
	return _locator->getCcd(ccdname);
}

// GuiderPort
template<>
astro::camera::GuiderPortPtr	LocatorAdapter<astro::camera::GuiderPort>::get(
					const DeviceName& guiderportname) {
        return getCameraChild(guiderportname);
}

template<>
astro::camera::GuiderPortPtr	LocatorAdapter<astro::camera::GuiderPort>::get0(
					const DeviceName& guiderportname) {
        return _locator->getGuiderPort0(guiderportname);
}

// FilterWheel
template<>
astro::camera::FilterWheelPtr	LocatorAdapter<astro::camera::FilterWheel>::get(
					const DeviceName& filterwheelname) {
	return getCameraChild(filterwheelname);
}

template<>
astro::camera::FilterWheelPtr	LocatorAdapter<astro::camera::FilterWheel>::get0(
					const DeviceName& filterwheelname) {
        return _locator->getFilterWheel(filterwheelname);
}

// CoolerPtr
template<>
astro::camera::CoolerPtr	LocatorAdapter<astro::camera::Cooler>::get(
					const DeviceName& coolername) {
	try {
		DeviceName	ccdname = coolername.parent(DeviceName::Cooler);
		CcdPtr	ccd = _locator->getCcd(ccdname);
		return ccd->getCooler();
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler '%s' not found",
			std::string(coolername).c_str());
	}
	return _locator->getCooler(coolername);
}

template<>
astro::camera::CoolerPtr	LocatorAdapter<astro::camera::Cooler>::get0(
					const DeviceName& name) {
        return _locator->getCooler(name);
}

// Focuser
template<>
astro::camera::FocuserPtr	LocatorAdapter<astro::camera::Focuser>::get(
					const DeviceName& name) {
        return _locator->getFocuser(name);
}

template<>
astro::camera::FocuserPtr	LocatorAdapter<astro::camera::Focuser>::get0(
					const DeviceName& name) {
        return _locator->getFocuser(name);
}

// Mount
template<>
astro::device::MountPtr	LocatorAdapter<astro::device::Mount>::get(
					const DeviceName& name) {
	return _locator->getMount(name);
}

template<>
astro::device::MountPtr	LocatorAdapter<astro::device::Mount>::get0(
					const DeviceName& name) {
	return _locator->getMount(name);
}

} // namespace device
} // namespace astro

