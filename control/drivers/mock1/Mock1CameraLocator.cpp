/*
 * mock1.cpp -- library structure
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <includes.h>
#include <Mock1Camera.h>
#include <iostream>

using namespace astro::camera;
using namespace astro::device;

namespace astro {
namespace camera {
namespace mock1 {

/**
 * \brief The Mock1 CameraLocator class
 *
 * The mock1 module offers a set of ten mock cameras each with two
 * CCDs with different image sizes.
 */
class Mock1CameraLocator : public DeviceLocator {
public:
	Mock1CameraLocator() { }
	virtual ~Mock1CameraLocator() { }
	virtual std::string	getName() const { return "module:mock1"; }
	virtual std::string	getVersion() const { return VERSION; }
	virtual std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
};

static std::string	cameraname(int id) {
	char	cameraname[20];
	snprintf(cameraname, sizeof(cameraname), "camera:mock1/%d", id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d -> %s", id, cameraname);
	return std::string(cameraname);
}

/**
 * \brief Get a list of cameras
 */
std::vector<std::string>	Mock1CameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	result;
	if (device != DeviceName::Camera) {
		return result;
	}
	for (int i = 0; i < 10; i++) {
		result.push_back(cameraname(i));
	}
	return result;
}

/**
 * \brief Get a camera by name
 */
CameraPtr	Mock1CameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mock1 camera: %s", sname.c_str());
	DeviceName	devname(name);
	int	id = stoi(devname.unitname());
	if (cameraname(id) != sname) {
		throw std::range_error("no such camera");
	}
	if ((id > 9) || (id < 0)) {
		throw std::range_error("no such camera");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create camera %d", id);
	return CameraPtr(new Mock1Camera(id));
}


} // namespace mock1
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::mock1::Mock1CameraLocator();
}
