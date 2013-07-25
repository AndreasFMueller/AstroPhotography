/*
 * DeviceLocator.cpp -- Device locator base class
 *
 * (c) 2013 Prof Dr Andreas Mueller,
 */
#include <AstroDevice.h>

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

astro::camera::CameraPtr	DeviceLocator::getCamera(const std::string& name) {
	throw std::runtime_error("base class does not implement cameras");
}

astro::camera::GuiderPortPtr	DeviceLocator::getGuiderPort(const std::string& name) {
	throw std::runtime_error("base class does not implement guider ports");
}

} // namespace device
} // namespace astro
