/*
 * SxLocator.cpp -- camera locator class for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxLocator.h>
#include <SxCamera.h>
#include <SxUtils.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <includes.h>

using namespace astro::usb;

namespace astro {
namespace module {
namespace sx {

//////////////////////////////////////////////////////////////////////
// Implementation of the Starlight Express Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      sx_name("sx");
static std::string      sx_version(VERSION);

/**
 * \brief Module descriptor for the Starlight express module
 */
class SxDescriptor : public ModuleDescriptor {
public:
	SxDescriptor() { }
	~SxDescriptor() { }
        virtual std::string     name() const {
                return sx_name;
        }
        virtual std::string     version() const {
                return sx_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace sx
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::sx::SxDescriptor();
}

namespace astro {
namespace camera {
namespace sx {

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for Starlight Express
//////////////////////////////////////////////////////////////////////

#define	SX_VENDOR_ID	0x1278

SxCameraLocator::SxCameraLocator() {
	context.setDebugLevel(3);
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
std::vector<std::string>	SxCameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;
	if (device != DeviceName::Camera) {
		return names;
	}
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
CameraPtr	SxCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing '%s'", sname.c_str());
	// parse the name string
	int	busnumber, deviceaddress;
	sscanf(sname.c_str(), "sx:%d:%d:", &busnumber, &deviceaddress);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has bus=%d, addr=%d", sname.c_str(),
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

} // namespace sx
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::sx::SxCameraLocator();
}
