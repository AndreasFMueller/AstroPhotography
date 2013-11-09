/*
 * DeviceLocator.cpp -- Device locator base class
 *
 * (c) 2013 Prof Dr Andreas Mueller,
 */
#include <AstroLocator.h>
#include <AstroCamera.h>

using namespace astro::camera;

namespace astro {
namespace device {

DeviceLocator::DeviceLocator() {
}

DeviceLocator::~DeviceLocator() {
}

std::string	DeviceLocator::getName() const {
	return std::string("generic device");
}

std::string	DeviceLocator::getVersion() const {
	return std::string(VERSION);
}

std::vector<std::string>	DeviceLocator::getDevicelist(const DeviceName::device_type device) {
	std::vector<std::string>	devices;
	return devices;
}

astro::camera::CameraPtr	DeviceLocator::getCamera0(const DeviceName& name) {
	throw std::runtime_error("cameras not implemented");
}

astro::camera::CcdPtr	DeviceLocator::getCcd0(const DeviceName& name) {
	throw std::runtime_error("ccds not implemented");
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort0(const DeviceName& name) {
	throw std::runtime_error("guiderport not implemented");
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel0(const DeviceName& name) {
	throw std::runtime_error("filterwheel not implemented");
}

astro::camera::CoolerPtr	DeviceLocator::getCooler0(const DeviceName& name) {
	throw std::runtime_error("cooler not implemented");
}

astro::camera::FocuserPtr	DeviceLocator::getFocuser0(const DeviceName& name) {
	throw std::runtime_error("focuser not implemented");
}

astro::camera::CameraPtr	DeviceLocator::getCamera(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getCamera(%s)", name.c_str());
	// check that the device name really is a camera name
	DeviceName	devname(name);
	if (!devname.hasType(DeviceName::Camera)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s is not a camera name",
			std::string(devname).c_str());
		throw std::invalid_argument("not a camera name");
	}
	// find the camera
	CameraPtr	camera;
	if (cameracache.find(name) == cameracache.end()) {
		camera = this->getCamera0(DeviceName(name));
		cameracache.insert(std::make_pair(name, camera));
	} else {
		camera = cameracache.find(name)->second;
	}
	return camera;
}

astro::camera::CcdPtr	DeviceLocator::getCcd(const std::string& name) {
	// check that the device name really is a ccd name
	DeviceName	devname(name);
	if (!devname.hasType(DeviceName::Ccd)) {
		throw std::invalid_argument("not a ccd name");
	}
	// find the ccd
	CcdPtr	ccd;
	if (ccdcache.find(name) == ccdcache.end()) {
		ccd = this->getCcd0(DeviceName(name));
		ccdcache.insert(std::make_pair(name, ccd));
	} else {
		ccd = ccdcache.find(name)->second;
	}
	return ccd;
}

astro::camera::CameraPtr	DeviceLocator::getCamera(size_t index) {
	std::vector<std::string>	cameras = this->getDevicelist();
	if (index >= cameras.size()) {
		throw std::runtime_error("cannot create a camera from an index");
	}
	return getCamera(cameras[index]);
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort(const std::string& name) {
	// check that the device name really is a Guiderport name
	DeviceName	devname(name);
	if (!devname.hasType(DeviceName::Guiderport)) {
		throw std::invalid_argument("not a guiderport name");
	}
	// get the guideport
	GuiderPortPtr	guiderport;
	if (guiderportcache.find(name) == guiderportcache.end()) {
		guiderport = this->getGuiderPort0(DeviceName(name));
		guiderportcache.insert(std::make_pair(name, guiderport));
	} else {
		guiderport = guiderportcache.find(name)->second;
	}
	return guiderport;
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel(const std::string& name) {
	// check that the device name really is a FilterWheel name
	DeviceName	devname(name);
	if (!devname.hasType(DeviceName::Filterwheel)) {
		throw std::invalid_argument("not a filterwheel name");
	}
	// get the filterwheel
	FilterWheelPtr	filterwheel;
	if (filterwheelcache.find(name) == filterwheelcache.end()) {
		filterwheel = this->getFilterWheel0(name);
		filterwheelcache.insert(std::make_pair(name, filterwheel));
	} else {
		filterwheel = filterwheelcache.find(name)->second;
	}
	return filterwheel;
}

astro::camera::CoolerPtr	DeviceLocator::getCooler(const std::string& name) {
	// check that the device name really is a Cooler name
	DeviceName	devname(name);
	if (!devname.hasType(DeviceName::Cooler)) {
		throw std::invalid_argument("not a cooler name");
	}
	// get the cooler
	CoolerPtr	cooler;
	if (coolercache.find(name) == coolercache.end()) {
		cooler = this->getCooler0(DeviceName(name));
		coolercache.insert(std::make_pair(name, cooler));
	} else {
		cooler = coolercache.find(name)->second;
	}
	return cooler;
}

astro::camera::FocuserPtr	DeviceLocator::getFocuser(const std::string& name) {
	// check that the device name really is a Focuser name
	DeviceName	devname(name);
	if (!devname.hasType(DeviceName::Focuser)) {
		throw std::invalid_argument("not a focuser name");
	}
	// get the focuser
	FocuserPtr	focuser;
	if (focusercache.find(name) == focusercache.end()) {
		focuser = this->getFocuser0(DeviceName(name));
		focusercache.insert(std::make_pair(name, focuser));
	} else {
		focuser = focusercache.find(name)->second;
	}
	return focuser;
}

template<>
astro::camera::CameraPtr	LocatorAdapter<astro::camera::Camera>::get(
					const std::string& name) {
        return _locator->getCamera(name);
}

template<>
astro::camera::CcdPtr	LocatorAdapter<astro::camera::Ccd>::get(
					const std::string& name) {
        return _locator->getCcd(name);
}

template<>
astro::camera::GuiderPortPtr	LocatorAdapter<astro::camera::GuiderPort>::get(
					const std::string& name) {
        return _locator->getGuiderPort(name);
}

template<>
astro::camera::FilterWheelPtr	LocatorAdapter<astro::camera::FilterWheel>::get(
					const std::string& name) {
        return _locator->getFilterWheel(name);
}

template<>
astro::camera::CoolerPtr	LocatorAdapter<astro::camera::Cooler>::get(
					const std::string& name) {
        return _locator->getCooler(name);
}

template<>
astro::camera::FocuserPtr	LocatorAdapter<astro::camera::Focuser>::get(
					const std::string& name) {
        return _locator->getFocuser(name);
}

} // namespace device
} // namespace astro

