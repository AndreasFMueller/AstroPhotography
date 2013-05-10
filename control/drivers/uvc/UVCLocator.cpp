/*
 * UVCLocator.cpp -- implementation of UVC camera locator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UVCLocator.h>
#include <UVCCamera.h>
#include <UVCUtils.h>
#include <debug.h>
#include <Format.h>

namespace astro {
namespace camera {
namespace uvc {

UVCCameraLocator::UVCCameraLocator() {
}

UVCCameraLocator::~UVCCameraLocator() {
}

std::string	UVCCameraLocator::getName() const {
	return "uvc";
}

std::string	UVCCameraLocator::getVersion() const {
	return VERSION;
}

std::vector<std::string>	UVCCameraLocator::getCameralist() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a camera list");
	std::vector<std::string>	cameras;
	// get a list of all device, then check whether they are UVC
	// devices
	std::vector<DevicePtr>	devices = context.devices();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "display the list");
	std::vector<DevicePtr>::iterator	i;
	for (i = devices.begin(); i != devices.end(); i++) {
		DevicePtr	deviceptr = *i;
		deviceptr->open();
		if (deviceptr->isVideoDevice()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found a video device");
			DeviceDescriptorPtr	descriptor
				= deviceptr->descriptor();
			std::string	name = stringprintf("uvc:%04x:%04x:%s",
				descriptor->idVendor(), descriptor->idProduct(),
				descriptor->iProduct().c_str());
			cameras.push_back(name);
		}
		deviceptr->close();
	}
	return cameras;
}

CameraPtr	UVCCameraLocator::getCamera(const std::string& name) {
	// extract the vendor id and the product id from the name and
	// open the device for it
	unsigned int	vendor, product;
	sscanf(name.c_str(), "uvc:%x:%x:", &vendor, &product);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening device %04x:%4x",
		vendor, product);

	// now scan the device lift 
	std::vector<DevicePtr>	devices = context.devices();
	std::vector<DevicePtr>::iterator	i;
	for (i = devices.begin(); i != devices.end(); i++) {
		DevicePtr	deviceptr = *i;
		DeviceDescriptorPtr	descriptor = (*i)->descriptor();
		if ((vendor == descriptor->idVendor())
			&& (product == descriptor->idProduct())) {
			return CameraPtr(new UVCCamera(deviceptr));
		}
	}
	throw UVCError("device not found");
}

CameraPtr	UVCCameraLocator::getCamera(size_t index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening camera %d", index);
	std::vector<DevicePtr>	devices = context.devices();
	std::vector<DevicePtr>::iterator	i;
	unsigned int	counter = 0;
	for (i = devices.begin(); i != devices.end(); i++) {
		DevicePtr	deviceptr = *i;
		deviceptr->open();
		if (deviceptr->isVideoDevice()) {
			if (index == counter) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "camera found");
				return CameraPtr(new UVCCamera(deviceptr));
			}
			counter++;
		}
		deviceptr->close();
	}
	throw UVCError("device not found");
}

} // namespace uvc
} // namespace camera
} // namespace astro

extern "C"
astro::camera::CameraLocator	*getCameraLocator() {
	return new astro::camera::uvc::UVCCameraLocator();
}

