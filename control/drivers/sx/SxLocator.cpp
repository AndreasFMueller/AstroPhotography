/*
 * SxLocator.cpp -- camera locator class for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxLocator.h>
#include <SxCamera.h>
#include <SxUtils.h>
#include <SxAO.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroUtils.h>
#include <includes.h>
#include <AstroExceptions.h>

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

	// list all devices from the context
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		// try to open the device. On Mac OS X, opening doesn't fail
		// ever, but on Linux, we may not have permission to open
		// all devices
		try {
			DevicePtr	devptr = *i;
			devptr->open();
			try {
				if (device == DeviceName::Camera) {
					names.push_back("camera:sx/" + sxname(devptr));
				}
				if (device == DeviceName::Guiderport) {
					names.push_back("guiderport:sx/" + sxname(devptr) + "/guiderport");
				}
			} catch (std::runtime_error& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "found a non"
					" SX device: %s", x.what());
			}
			devptr->close();
		} catch (std::exception& x) {
			// log the error, but don't do anything about it
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot work with device");
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d SX devices", names.size());
	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	SxCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name.unitname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "analyzing '%s'", sname.c_str());
	// parse the name string
	int	busnumber = 0, deviceaddress = 0;
	sxparse(sname, busnumber, deviceaddress);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"looking for busnumber=%d, deviceaddress=%d",
		busnumber, deviceaddress);

	// find the device with this bus number and address
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		DevicePtr	dptr = (*i);
		if ((dptr->getBusNumber() == busnumber) &&
			(dptr->getDeviceAddress() == deviceaddress)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found matching device");
			dptr->open();
			return CameraPtr(new SxCamera(dptr));
		}
	}
	throw SxError("cannot create a camera from a name");
}

/**
 * \brief Get an AO object
 */
AdaptiveOpticsPtr	SxCameraLocator::getAdaptiveOptics0(const DeviceName& name) {
	std::string	sname = URL::decode(name.unitname());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AO unit device name: %s", sname.c_str());
	return AdaptiveOpticsPtr(new SxAO(sname));
}

/**
 * \brief Get a guider port by name
 */
GuiderPortPtr	SxCameraLocator::getGuiderPort0(const DeviceName& name) {
	DeviceName	cameraname(DeviceName::Camera, name);
	cameraname.pop_back();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for guider port of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	if (!camera->hasGuiderPort()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera has no guider port");
		throw NotFound("camera does not have guider port");
	}
	return camera->getGuiderPort();
}

} // namespace sx
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::sx::SxCameraLocator();
}
