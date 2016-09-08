/*
 * AstroLocator.h -- DeviceLocator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroLocator_h
#define _AstroLocator_h

#include <AstroCamera.h>
#include <map>
#include <stdexcept>

namespace astro {
namespace device {

/**
 * \brief Device cache adapter serving devices of any type
 *
 * 
 */
class DeviceLocator;

template<typename Device>
class DeviceCacheAdapter {
	DeviceLocator	*_locator;
public:
	DeviceCacheAdapter(DeviceLocator *locator) : _locator(locator) { }
	typename Device::sharedptr	get0(const DeviceName& name);
};

template<>
astro::camera::AdaptiveOpticsPtr
DeviceCacheAdapter<astro::camera::AdaptiveOptics>::get0(const DeviceName& name);

template<>
astro::camera::CameraPtr
DeviceCacheAdapter<astro::camera::Camera>::get0(const DeviceName& name);

template<>
astro::camera::CcdPtr
DeviceCacheAdapter<astro::camera::Ccd>::get0(const DeviceName& name);

template<>
astro::camera::CoolerPtr
DeviceCacheAdapter<astro::camera::Cooler>::get0(const DeviceName& name);

template<>
astro::camera::FilterWheelPtr
DeviceCacheAdapter<astro::camera::FilterWheel>::get0(const DeviceName& name);

template<>
astro::camera::FocuserPtr
DeviceCacheAdapter<astro::camera::Focuser>::get0(const DeviceName& name);

template<>
astro::camera::GuidePortPtr
DeviceCacheAdapter<astro::camera::GuidePort>::get0(const DeviceName& name);

template<>
astro::device::MountPtr
DeviceCacheAdapter<astro::device::Mount>::get0(const DeviceName& name);

/**
 * \brief Cache for devices
 *
 * Devices are cached by name. This template implements a cache for
 * each type of object.
 */
template<typename Device>
class DeviceCache {
	std::map<std::string, typename Device::sharedptr>	_cache;
	DeviceLocator	*_locator;
public:
	DeviceCache(DeviceLocator *locator) : _locator(locator) { }
	typename Device::sharedptr	get(const std::string& name);
};

template<typename Device>
typename Device::sharedptr	DeviceCache<Device>::get(const std::string& name) {
	DeviceName	devname(name);
	if (!devname.hasType(Device::devicetype)) {
		std::string	t = DeviceName::type2string(Device::devicetype);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s is not of type %s",
			name.c_str(), t.c_str());
		throw std::invalid_argument("name does not refer the "
			"right device type");
	}
	typename Device::sharedptr	device;
	if (_cache.find(name) == _cache.end()) {
		DeviceCacheAdapter<Device>	dca(_locator);
		device = dca.get0(name);
		_cache.insert(std::make_pair(name, device));
	} else {
		device = _cache.find(name)->second;
	}
	return device;
}

/**
 * \brief The device locator can locate device within a module
 *
 * The device locator keeps a cache of all devices ever retrieved from this
 * locator, which ensures that 
 */
class   DeviceLocator {
	DeviceCache<astro::camera::AdaptiveOptics>	aocache;
	DeviceCache<astro::camera::Camera>	cameracache;
	DeviceCache<astro::camera::Ccd>		ccdcache;
	DeviceCache<astro::camera::Cooler>	coolercache;
	DeviceCache<astro::camera::FilterWheel>	filterwheelcache;
	DeviceCache<astro::camera::Focuser>	focusercache;
	DeviceCache<astro::camera::GuidePort>	guideportcache;
	DeviceCache<astro::device::Mount>	mountcache;
public:
	virtual astro::camera::AdaptiveOpticsPtr	getAdaptiveOptics0(const DeviceName& name);
	virtual	astro::camera::CameraPtr	getCamera0(const DeviceName& name);
	virtual astro::camera::CcdPtr		getCcd0(const DeviceName& name);
	virtual	astro::camera::CoolerPtr	getCooler0(const DeviceName& name);
	virtual	astro::camera::FilterWheelPtr	getFilterWheel0(const DeviceName& name);
	virtual	astro::camera::FocuserPtr	getFocuser0(const DeviceName& name);
	virtual	astro::camera::GuidePortPtr	getGuidePort0(const DeviceName& name);
	virtual	astro::device::MountPtr		getMount0(const DeviceName& name);
public:
	DeviceLocator();
	virtual ~DeviceLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(
		const DeviceName::device_type device = DeviceName::Camera);
	std::vector<DeviceName>	getDeviceList(
		const DeviceName::device_type device = DeviceName::Camera);
	astro::camera::AdaptiveOpticsPtr	getAdaptiveOptics(const std::string& name);
	astro::camera::CameraPtr	getCamera(const std::string& name);
	astro::camera::CameraPtr	getCamera(size_t index);
	astro::camera::CcdPtr		getCcd(const std::string& name);
	astro::camera::GuidePortPtr	getGuidePort(const std::string& name);
	astro::camera::FilterWheelPtr	getFilterWheel(const std::string& name);
	astro::camera::CoolerPtr	getCooler(const std::string& name);
	astro::camera::FocuserPtr	getFocuser(const std::string& name);
	astro::device::MountPtr		getMount(const std::string& name);
};

typedef std::shared_ptr<DeviceLocator>	DeviceLocatorPtr;

//////////////////////////////////////////////////////////////////////
// adapter class for DeviceLocator to extract objects of a given type
//////////////////////////////////////////////////////////////////////
template<typename device>
class LocatorAdapter {
        astro::device::DeviceLocatorPtr _locator;
public:
        LocatorAdapter(astro::device::DeviceLocatorPtr locator)
                : _locator(locator) { }
        typename device::sharedptr      get0(const DeviceName& name);
        typename device::sharedptr      get(const DeviceName& name);
	typename device::sharedptr	getCameraChild(const DeviceName& name);
};

template<typename device>
typename device::sharedptr
LocatorAdapter<device>::getCameraChild(const DeviceName& name) {
	try {
		DeviceName      devicename(name);
		DeviceName      cameraname
			= devicename.parent(DeviceName::Camera);
		astro::camera::CameraPtr	camera
			= _locator->getCamera(cameraname);
		astro::camera::CameraDeviceAdapter<device>     cda(camera);
		return cda.get(devicename);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot find camera child: %s",
			std::string(name).c_str());
	}
	return get0(name);
}

template<>
astro::camera::AdaptiveOpticsPtr
LocatorAdapter<astro::camera::AdaptiveOptics>::get0(const DeviceName& name);

template<>
astro::camera::CameraPtr
LocatorAdapter<astro::camera::Camera>::get0(const DeviceName& name);

template<>
astro::camera::CcdPtr
LocatorAdapter<astro::camera::Ccd>::get0(const DeviceName& name);

template<>
astro::camera::GuidePortPtr
LocatorAdapter<astro::camera::GuidePort>::get0(const DeviceName& name);

template<>
astro::camera::FilterWheelPtr
LocatorAdapter<astro::camera::FilterWheel>::get0(const DeviceName& name);

template<>
astro::camera::CoolerPtr
LocatorAdapter<astro::camera::Cooler>::get0(const DeviceName& name);

template<>
astro::camera::FocuserPtr
LocatorAdapter<astro::camera::Focuser>::get0(const DeviceName& name);

template<>
astro::device::MountPtr
LocatorAdapter<astro::device::Mount>::get0(const DeviceName& name);


template<>
astro::camera::AdaptiveOpticsPtr
LocatorAdapter<astro::camera::AdaptiveOptics>::get(const DeviceName& name);

template<>
astro::camera::CameraPtr
LocatorAdapter<astro::camera::Camera>::get(const DeviceName& name);

template<>
astro::camera::CcdPtr
LocatorAdapter<astro::camera::Ccd>::get(const DeviceName& name);

template<>
astro::camera::GuidePortPtr
LocatorAdapter<astro::camera::GuidePort>::get(const DeviceName& name);

template<>
astro::camera::FilterWheelPtr
LocatorAdapter<astro::camera::FilterWheel>::get(const DeviceName& name);

template<>
astro::camera::CoolerPtr
LocatorAdapter<astro::camera::Cooler>::get(const DeviceName& name);

template<>
astro::camera::FocuserPtr
LocatorAdapter<astro::camera::Focuser>::get(const DeviceName& name);

template<>
astro::device::MountPtr
LocatorAdapter<astro::device::Mount>::get(const DeviceName& name);

} // namespace device
} // namespace astro

#endif /* _AstroLocator_h */
