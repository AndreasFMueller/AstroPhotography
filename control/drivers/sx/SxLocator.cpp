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

typedef struct sx_hascooler_s {
	unsigned short	product;
	bool	has_cooler;
} sx_hascooler_t;

#define sx_hascooler_size	20
static sx_hascooler_t	models[sx_hascooler_size] {
	{ 0x0000, false },
	{ 0x0100, true },
	{ 0x0105, true },
	{ 0x0107, true },
	{ 0x0119, true },
	{ 0x0126, true },
	{ 0x0128, true },
	{ 0x0135, true },
	{ 0x0136, true },
	{ 0x0200, false },
	{ 0x0305, true },
	{ 0x0307, true },
	{ 0x0308, true },
	{ 0x0309, true },
	{ 0x0319, true },
	{ 0x0325, true },
	{ 0x0326, true },
	{ 0x0507, false }, // Lodestar
	{ 0x0509, false }, // Oculus
	{ 0x0517, false }, // Costar
};
static bool	has_cooler(unsigned short product) {
	for (int i = 0; i < sx_hascooler_size; i++) {
		if (models[i].product == product) {
			return models[i].has_cooler;
		}
	}
	return false;
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

static void	addname(std::vector<std::string>& names, usb::DevicePtr devptr,
	DeviceName::device_type device) {
	SxName sxname(devptr);
	switch (device) {
	case DeviceName::Camera:
		names.push_back(sxname.cameraname());
		break;
	case DeviceName::Ccd:
		names.push_back(sxname.ccdname());
		break;
	case DeviceName::Cooler:
		if (has_cooler(sxname.idproduct())) {
			names.push_back(sxname.coolername());
		}
		break;
	case DeviceName::Guiderport:
		names.push_back(sxname.guiderportname());
		break;
	default:
		// unknown components
		break;
	}
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	SxCameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;

	// special treatment for adaptive optics devices. These devices
	// are not discoverable, so their names must be retrieved from
	// the properties file
	if (device == DeviceName::AdaptiveOptics) {
		for (int unit = 0; unit < 4; unit++) {
			std::string	devicename
				= stringprintf("adaptiveoptics:sx/%d", unit);
			Properties	properties(devicename);
			if (properties.hasProperty("device")) {
				names.push_back(devicename);
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d adaptiveoptics units",
			names.size());
		return names;
	}

	// list all devices from the context
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		// try to open the device. On Mac OS X, opening doesn't fail
		// ever, but on Linux, we may not have permission to open
		// all devices. We ignore devices that we cannot open
		try {
			DevicePtr	devptr = *i;
			devptr->open();
			try {
				addname(names, devptr, device);
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

	// return the list of devices
	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	SxCameraLocator::getCamera0(const DeviceName& name) {
	SxName	sxname(name);
	if (!sxname.isCamera(name)) {
		std::string	msg = stringprintf("%s is not a Camera name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// find the device with this bus number and address
	std::vector<DevicePtr>	d = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		DevicePtr	dptr = (*i);
		int	busnumber = dptr->getBusNumber();
		int	deviceaddress = dptr->getDeviceAddress();
		if ((busnumber == sxname.busnumber()) &&
			(deviceaddress == sxname.deviceaddress())) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found matching device");
			dptr->open();
			return CameraPtr(new SxCamera(dptr));
		}
	}
	std::string	msg = stringprintf("cannot create camera from '%s'",
		name.toString().c_str());
	throw SxError(msg.c_str());
}

/**
 * \brief Get a Cooler based on the cooler name
 *
 * \param name  name of the cooler
 */
CoolerPtr	SxCameraLocator::getCooler0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieveing cooler '%s'",
		name.toString().c_str());
	SxName	sxname(name);
	if (!sxname.isCooler(name)) {
		std::string	msg = stringprintf("%s is not a Cooler name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// get the camera with the same name
	DeviceName	cameraname = sxname.cameraname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	CcdPtr	ccd = camera->getCcd(0);
	if (!ccd->hasCooler()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "ccd has no cooler");
		throw NotFound("cooler not found");
	}
	return ccd->getCooler();
}

CcdPtr	SxCameraLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieveing CCD '%s'",
		name.toString().c_str());
	SxName	sxname(name);
	if (!sxname.isCcd(name)) {
		std::string	msg = stringprintf("%s is not a CCD name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = sxname.cameraname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	CcdPtr	ccd = camera->getCcd(0);
	return ccd;
}

/**
 * \brief Get an AO object
 */
AdaptiveOpticsPtr	SxCameraLocator::getAdaptiveOptics0(const DeviceName& name) {
	return AdaptiveOpticsPtr(new SxAO(name));
}

/**
 * \brief Get a guider port by name
 */
GuiderPortPtr	SxCameraLocator::getGuiderPort0(const DeviceName& name) {
	SxName	sxname(name);
	if (!sxname.isGuiderport(name)) {
		std::string	msg = stringprintf("%s is not a Guiderport name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	DeviceName	cameraname = sxname.cameraname();
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
