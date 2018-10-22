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
//
// The DeviceCacheAdapter is essentially a forwarding service without
// any logic, so we lave the locking stuff to the Device Cache and
// the DeviceLocator, who do the real word
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
GuidePortPtr	DeviceCacheAdapter<GuidePort>::get0(const DeviceName& name) {
	return _locator->getGuidePort0(name);
}

template<>
MountPtr	DeviceCacheAdapter<Mount>::get0(const DeviceName& name) {
	return _locator->getMount0(name);
}

//////////////////////////////////////////////////////////////////////
// DeviceLocator implementation
//////////////////////////////////////////////////////////////////////
DeviceLocator::DeviceLocator() :
	aocache(this, this->_mutex),
	cameracache(this, this->_mutex),
	ccdcache(this, this->_mutex),
	coolercache(this, this->_mutex),
	filterwheelcache(this, this->_mutex),
	focusercache(this, this->_mutex),
	guideportcache(this, this->_mutex),
	mountcache(this, this->_mutex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing DeviceLocator %s at %p",
		demangle(typeid(*this).name()).c_str(), this);
}

DeviceLocator::~DeviceLocator() {
}

std::string	DeviceLocator::getName() const {
	return std::string("generic device");
}

std::string	DeviceLocator::getVersion() const {
	return std::string(VERSION);
}

/**
 * \brief Return list of devices
 *
 * The base class always returns an empty list
 */
std::vector<std::string>	DeviceLocator::getDevicelist(
					const DeviceName::device_type) {
	std::vector<std::string>	devices;
	return devices;
}

/**
 * \brief Return a list of device names as DeviceName objects
 *
 * Note that there is only a very slight difference to the method that
 * returns a list of string names: getDeviceList <-> getDevicelist
 */
std::vector<DeviceName>	DeviceLocator::getDeviceList(
					const DeviceName::device_type device) {
	std::vector<DeviceName>	devices;
	std::vector<std::string>	l = getDevicelist(device);
	for (auto i = l.begin(); i != l.end(); i++) {
		devices.push_back(DeviceName(*i));
	}
	return devices;
}

astro::camera::AdaptiveOpticsPtr	DeviceLocator::getAdaptiveOptics0(const DeviceName&) {
	throw std::runtime_error("adaptive optics not implemented");
}

astro::camera::CameraPtr	DeviceLocator::getCamera0(const DeviceName&) {
	throw std::runtime_error("cameras not implemented");
}

/**
 * \brief Get a CCD
 *
 * The base class knows how to handle the case where a CCD is a direct
 * parent of a camera. The last component of a CCD name is the name of
 * the ccd. This method cuts of the ccd name at the end of the device
 * name and turns it into a camera name. It then uses the locator to
 * get a camera of this name (here it becomes really important that
 * getCamera() properly locks the data structures). Finally it uses the
 * getCcd() method of the camera to retrieve the camera
 */
astro::camera::CcdPtr	DeviceLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find ccd %s", name.toString().c_str());

	// first construct a comera name and retrieve the camera. As as side
	// effect, this puts the camera into the cache
	DeviceName	cameraname = name.parent(DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);

	// now scan all the CCDs of the camera to find a matching ccd
	int	nccds = camera->nCcds();
	for (int i = 0; i < nccds; i++) {
		CcdInfo	info = camera->getCcdInfo(i);
		if (info.name() == name) {
			return camera->getCcd(i);
		}
	}

	// if we get to this point, then a CCD of that name does
	// not exist
	throw NotFound(stringprintf("ccd %s not found",
		name.toString().c_str()));
}

/**
 * \brief Get a GuidePort
 *
 * As for getCcd0, this method implements the case where the guideport
 * has is a direct child of the camera, but a different type.
 */
astro::camera::GuidePortPtr	DeviceLocator::getGuidePort0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find guideport %s",
		name.toString().c_str());
	// construct the camera name as an immediate parent of the device
	// name, then search for the camera.  As a side effect, this puts
	// the camera into the cache.
	DeviceName	cameraname = name.parent(DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);

	// return the guide port if it exists
	if (camera->hasGuidePort()) {
		return camera->getGuidePort();
	}
	throw NotFound(stringprintf("guideport %s not found",
		name.toString().c_str()));
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel0(const DeviceName& name) {
	std::string	msg = stringprintf("getFilterWheel0(%s) not implemented",
		name.toString().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get a cooler
 *
 * The base class assumes that coolers have the some names as CCDs, just
 * with a different type.
 */
astro::camera::CoolerPtr	DeviceLocator::getCooler0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "find cooler %s",
		name.toString().c_str());
	// construct the camera name as an immediate parent of the device
	// name, then search for the ccd.  As a side effect, this puts
	// the ccd into the cache.
	DeviceName	ccdname = name.parent(DeviceName::Ccd);
	CcdPtr	ccd = this->getCcd(ccdname);

	// now get the cooler from the CCD
	if (ccd->hasCooler()) {
		return ccd->getCooler();
	}

	// if we get here, then there is no such cooler
	throw NotFound(stringprintf("cooler %s not found",
		name.toString().c_str()));
}

/**
 * \brief No default method to get a focuser
 */
astro::camera::FocuserPtr	DeviceLocator::getFocuser0(const DeviceName& name) {
	std::string	msg = stringprintf("getFocuser0(%s) not implemented",
		name.toString().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief No default method to get a mount
 */
astro::device::MountPtr	DeviceLocator::getMount0(const DeviceName& name) {
	std::string	msg = stringprintf("getMount0(%s) not implemented",
		name.toString().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get an adaptive optics device
 */
astro::camera::AdaptiveOpticsPtr	DeviceLocator::getAdaptiveOptics(
						const std::string& name) {
	return aocache.get(name);
}

/**
 * \brief Get a camera
 */
astro::camera::CameraPtr	DeviceLocator::getCamera(
						const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera %s requested", name.c_str());
	return cameracache.get(name);
}

/**
 * \brief Get a ccd
 */
astro::camera::CcdPtr	DeviceLocator::getCcd(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd %s requested", name.c_str());
	return ccdcache.get(name);
}

/**
 * \brief Get a cooler
 */
astro::camera::CoolerPtr	DeviceLocator::getCooler(
					const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cooler %s requested", name.c_str());
	return coolercache.get(name);
}

/**
 * \brief Get a camera
 *
 * \param index		index of the camera in the camera list
 */
astro::camera::CameraPtr	DeviceLocator::getCamera(size_t index) {
	std::vector<std::string>	cameras = this->getDevicelist();
	if (index >= cameras.size()) {
		std::string	msg = stringprintf("index %d to large (max %d)",
			index, cameras.size());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	return getCamera(cameras[index]);
}

/**
 * \brief Get a filterwheel
 */
astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel(
					const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel %s requested",
		name.c_str());
	return filterwheelcache.get(name);
}

/**
 * \brief Get a focuser
 */
astro::camera::FocuserPtr	DeviceLocator::getFocuser(
					const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser %s requested", name.c_str());
	return focusercache.get(name);
}

/**
 * \brief Get a guide port
 */
astro::camera::GuidePortPtr	DeviceLocator::getGuidePort(
					const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guideport %s requested", name.c_str());
	return guideportcache.get(name);
}

/**
 * \brief Get a mount
 */
astro::device::MountPtr	DeviceLocator::getMount(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mount %s requested", name.c_str());
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

// GuidePort
template<>
astro::camera::GuidePortPtr	LocatorAdapter<astro::camera::GuidePort>::get(
					const DeviceName& guideportname) {
        return getCameraChild(guideportname);
}

template<>
astro::camera::GuidePortPtr	LocatorAdapter<astro::camera::GuidePort>::get0(
					const DeviceName& guideportname) {
        return _locator->getGuidePort0(guideportname);
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

