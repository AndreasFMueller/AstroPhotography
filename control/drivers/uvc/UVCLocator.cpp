/*
 * UvcLocator.cpp -- implementation of UVC camera locator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UvcLocator.h>
#include <UvcCamera.h>
#include <UvcUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroLoader.h>
#include <config.h>

using namespace astro::device;

//////////////////////////////////////////////////////////////////////
// SBIG Module Descriptor
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace module {
namespace uvc {

static std::string      uvc_name("uvc");
static std::string      uvc_version(VERSION);

/**
 * \brief Module descriptor for the USB Video Class module
 */
class UvcDescriptor : public ModuleDescriptor {
public:
	UvcDescriptor() { }
	~UvcDescriptor() { }
	virtual std::string     name() const {
		return uvc_name;
	}
	virtual std::string     version() const {
		return uvc_version;
	}
	virtual bool    hasDeviceLocator() const {
		return true;
	}
};


} // namespace uvc
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor *getDescriptor() {
	return new astro::module::uvc::UvcDescriptor();
}

//////////////////////////////////////////////////////////////////////
// UvcCameraLocator implementation
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace camera {
namespace uvc {

UvcCameraLocator::UvcCameraLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating the UVC locator");
}

UvcCameraLocator::~UvcCameraLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying the UVC locator");
}

std::string	UvcCameraLocator::getName() const {
	return "uvc";
}

std::string	UvcCameraLocator::getVersion() const {
	return VERSION;
}

std::vector<std::string>	UvcCameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	cameras;
	if (device != DeviceName::Camera) {
		return cameras;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a camera list");
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
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "not a video device");
		}
		deviceptr->close();
	}
	return cameras;
}

CameraPtr	UvcCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	// extract the vendor id and the product id from the name and
	// open the device for it
	unsigned int	vendor, product;
	sscanf(sname.c_str(), "uvc:%x:%x:", &vendor, &product);
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
			return CameraPtr(new UvcCamera(deviceptr));
		}
	}
	throw UvcError("device not found");
}

} // namespace uvc
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::uvc::UvcCameraLocator();
}

