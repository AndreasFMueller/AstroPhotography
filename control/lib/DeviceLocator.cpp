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

std::vector<std::string>	DeviceLocator::getDevicelist(device_type device) {
	std::vector<std::string>	devices;
	return devices;
}

astro::camera::CameraPtr	DeviceLocator::getCamera0(const std::string& name) {
	throw std::runtime_error("cameras not implemented");
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort0(const std::string& name) {
	throw std::runtime_error("guiderport not implemented");
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel0(const std::string& name) {
	throw std::runtime_error("filterwheel not implemented");
}

astro::camera::CameraPtr	DeviceLocator::getCamera(const std::string& name) {
	CameraPtr	camera;
	if (cameracache.find(name) == cameracache.end()) {
		camera = this->getCamera0(name);
		cameracache.insert(std::make_pair(name, camera));
	} else {
		camera = cameracache.find(name)->second;
	}
	return camera;
}

astro::camera::CameraPtr	DeviceLocator::getCamera(size_t index) {
	std::vector<std::string>	cameras = this->getDevicelist();
	if (index >= cameras.size()) {
		throw std::runtime_error("cannot create a camera from an index");
	}
	return getCamera(cameras[index]);
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort(const std::string& name) {
	GuiderPortPtr	guiderport;
	if (guiderportcache.find(name) == guiderportcache.end()) {
		guiderport = this->getGuiderPort0(name);
		guiderportcache.insert(std::make_pair(name, guiderport));
	} else {
		guiderport = guiderportcache.find(name)->second;
	}
	return guiderport;
}

astro::camera::FilterWheelPtr	DeviceLocator::getFilterWheel(const std::string& name) {
	FilterWheelPtr	filterwheel;
	if (filterwheelcache.find(name) == filterwheelcache.end()) {
		filterwheel = this->getFilterWheel0(name);
		filterwheelcache.insert(std::make_pair(name, filterwheel));
	} else {
		filterwheel = filterwheelcache.find(name)->second;
	}
	return filterwheel;
}

} // namespace device
} // namespace astro
