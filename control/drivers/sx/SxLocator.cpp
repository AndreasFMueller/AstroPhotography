/*
 * SxLocator.cpp -- camera locator class for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxLocator.h>
#include <SxCamera.h>
#include <SxUtils.h>
#include <Format.h>
#include <debug.h>
#include <includes.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

#define	SX_VENDOR_ID	0x1278

SxCameraLocator::SxCameraLocator() {
	context.setDebugLevel(0);
}

SxCameraLocator::~SxCameraLocator() {
}

/**
 * \brief Get module name.
 */
std::string	SxCameraLocator::getName() const {
	return std::string("sx");
}

/**
 * \brief Get module version.
 */
std::string	SxCameraLocator::getVersion() const {
	return VERSION;
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	SxCameraLocator::getCameralist() {
	std::vector<std::string>	names;
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		// try to open the device. On Mac OS X, opening doesn't fail
		// ever, but on Linux, we may not have permission to open
		// all devices
		try {
			(*i)->open();
			DeviceDescriptorPtr	descriptor = (*i)->descriptor();
			if (SX_VENDOR_ID == descriptor->idVendor()) {
				std::string	name = stringprintf(
					"sx:%03d:%03d:%s:%04x:%04x:%s",
					(*i)->getBusNumber(),
					(*i)->getDeviceAddress(),
					descriptor->iProduct().c_str(),
					descriptor->idVendor(),
					descriptor->idProduct(),
					descriptor->iSerialNumber().c_str());
				names.push_back(name);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"SX device %s found", name.c_str());
			}
			(*i)->close();
		} catch (std::exception& x) {
			// log the error, but don't do anything about it
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot work with device");
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d SX cameras", names.size());
	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	SxCameraLocator::getCamera(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing '%s'", name.c_str());
	// parse the name string
	int	busnumber, deviceaddress;
	sscanf(name.c_str(), "sx:%d:%d:", &busnumber, &deviceaddress);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has bus=%d, addr=%d", name.c_str(),
		busnumber, deviceaddress);

	// find the device with this bus number and address
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		if (((*i)->getBusNumber() == busnumber) &&
			((*i)->getDeviceAddress() == deviceaddress)) {
			DevicePtr	dptr = (*i);
			return CameraPtr(new SxCamera(dptr));
		}
	}
	throw SxError("cannot create a camera from a name");
}

/**
 * \brief Get a camera pointer.
 *
 * This method assumes that the context always returns the connected
 * starlight express devices in the same order, so that the index
 * into the list of starlight express devices uniquely specifies a
 * camera.
 * \param index		index of the camera:
 * \return Camera with that index
 */
CameraPtr	SxCameraLocator::getCamera(size_t index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open device %d as SX camera", index);
	size_t	counter = 0;
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		DeviceDescriptorPtr	descriptor = (*i)->descriptor();
		if (SX_VENDOR_ID == descriptor->idVendor()) {
			if (index == counter) {
				DevicePtr	dptr = (*i);
				return CameraPtr(new SxCamera(dptr));
			}
			counter++;
		}
	}
	throw SxError("cannot create a camera from an index");
}

} // namespace sx
} // namespace camera
} // namespace astro

extern "C"
astro::camera::CameraLocator    *getCameraLocator() {
	return new astro::camera::sx::SxCameraLocator();
}

