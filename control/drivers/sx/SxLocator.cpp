/*
 * SxLocator.cpp -- camera locator class for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroUtils.h>
#include <includes.h>
#include <AstroExceptions.h>
#include "SxLocator.h"
#include "SxCamera.h"
#include "SxUtils.h"
#include "SxAO.h"
#include "SxFilterWheel.h"
#include "sx.h"

using namespace astro::usb;

namespace astro {
namespace module {
namespace sx {


//////////////////////////////////////////////////////////////////////
// Implementation of the Starlight Express Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      sx_name(SX_MODULE_NAME);
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

static std::once_flag   descriptor_once;
static astro::module::ModuleDescriptor  *descriptor;
void	setup_descriptor() {
	descriptor = new SxDescriptor();
}

} // namespace sx
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	std::call_once(astro::module::sx::descriptor_once,
		astro::module::sx::setup_descriptor);
	USBdebugEnable();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SxDescriptor: %p",
		astro::module::sx::descriptor);
	return astro::module::sx::descriptor;
}

namespace astro {
namespace camera {
namespace sx {

std::mutex	SxCameraLocator::_hid_mutex;

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for Starlight Express
//////////////////////////////////////////////////////////////////////

SxCameraLocator::SxCameraLocator() {
	//context.setDebugLevel(4);
}

SxCameraLocator::~SxCameraLocator() {
}

/**
 * \brief Get module name.
 */
std::string	SxCameraLocator::getName() const {
	return std::string(SX_MODULE_NAME);
}

/**
 * \brief Get module version.
 */
std::string	SxCameraLocator::getVersion() const {
	return VERSION;
}

/**
 * \brief Add a name to the map of SX usb devices
 *
 * This method is only used for camera related devices, not for AO or
 * filter wheels
 *
 * \param name		vector of device names
 * \param device	type of device to construct
 * \param devptr	USB device pointer
 */
void	SxCameraLocator::addname(std::vector<std::string> *names,
		DeviceName::device_type device, usb::DevicePtr devptr) {
	// construct the name of the device, and find its camera name
	SxName sxname(device, devptr);
	std::string	cameraname = sxname.cameraname();

	// now verify whether the device already exists
	if (_sxdevices.find(cameraname) == _sxdevices.end()) {
		// we don't have the device yet, so we store it in the
		// the device map
		_sxdevices.insert(std::make_pair(cameraname, devptr));
	}

	// we don't need to remember the name in the names if names is
	// a NULL pointer
	if (NULL == names) {
		return;
	}

	// store device names
	switch (device) {
	case DeviceName::Camera:
		names->push_back(sxname.cameraname());
		break;
	case DeviceName::Ccd:
		names->push_back(sxname.ccdname());
		break;
	case DeviceName::Cooler:
		if (sxname.hascooler()) {
			names->push_back(sxname.coolername());
		}
		break;
	case DeviceName::Guideport:
		names->push_back(sxname.guideportname());
		break;
	default:
		// unknown components
		break;
	}
}

/**
 * \brief Get the connection for a device name
 *
 * \param name	the name of the device
 */
usb::DevicePtr	SxCameraLocator::deviceForName(const DeviceName& name) {
	return deviceForName(name.enclosurename());
}

/**
 * \brief Get the connetion for a enclosure name
 *
 * \param name	the name of the enclosure
 */
usb::DevicePtr	SxCameraLocator::deviceForName(const std::string& enclosure) {
	// first isolate the enclosure name
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for enclosure %s",
		enclosure.c_str());

	// this function works on the device map, so we have to lock it
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// first check the cache
	auto	j = _sxdevices.find(enclosure);
	if (j != _sxdevices.end()) {
		// we have found the device we are interested 
		debug(LOG_DEBUG, DEBUG_LOG, 0, "returning device %s from map",
			enclosure.c_str());
		return j->second;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s not in the cache",
		enclosure.c_str());

	// get a list of devices
	std::vector<DevicePtr>	d = context.devices(SX_VENDOR_ID);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d devices", d.size());
	for (auto i = d.begin(); i != d.end(); i++) {
		// get the device
		DevicePtr	dptr = (*i);

		// open if necessare
		bool	needsclose = false;
		if (!dptr->isOpen()) {
			dptr->open();
			needsclose = true;
		}

		// get the device enclosure name
		std::string	devenclosure = SxName::deviceName(dptr);

		// close if we opened
		if (needsclose) {
			dptr->close();
		}

		// if the enclosure name matches remember and return the USB device
		if (enclosure == devenclosure) {
			_sxdevices.insert(std::make_pair(enclosure, dptr));
			return dptr;
		}
	}

	// throw an exception because the object was not found
	std::string	msg = stringprintf("device %s not found",
		enclosure.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotFound(msg);
}

/**
 * \brief Remove a device from the cache
 *
 * \param name	the device name to remove
 */
void	SxCameraLocator::forget(const DeviceName& name) {
	forget(name.enclosurename());
}

/**
 * \brief Remove a device from the cache by enclosure name
 *
 * \param enclosurename	the enclosure name to remove
 */
void	SxCameraLocator::forget(const std::string& enclosurename) {
	auto	j = _sxdevices.find(enclosurename);
	if (j != _sxdevices.end()) {
		_sxdevices.erase(j);
	}
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \param device	type of devices to list
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	SxCameraLocator::getDevicelist(
					DeviceName::device_type device) {
	std::vector<std::string>	names;

	// special treatment for adaptive optics devices. These devices
	// are not discoverable, so their names must be retrieved from
	// the properties file. There are at most 4 AO units for which
	// the serial device can be defined in the properties file.
	// The entry must be of the form
	//     adaptiveoptics:sx/0 device = /dev/ttyUSB0
	if (device == DeviceName::AdaptiveOptics) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "listing SX AOs");
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

	// special treatment for FilterWheel. The FilterWheels are not 
	// associated with cameras, so we have to scan for them separately
	// This version of the driver can handle only a single filter wheel.
	if (device == DeviceName::Filterwheel) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "listing SX Filterwheels");
		// enumerate filterwheels
		debug(LOG_DEBUG, DEBUG_LOG, 0, "scan for hid devices");

		// find out whether there is a matching product id, this
		// prevents a crash on macosx if there is no SX filterwheel
		// device present
		libusb_device_handle	*h = libusb_open_device_with_vid_pid(
			NULL, SX_VENDOR_ID, SX_FILTERWHEEL_PRODUCT_ID);
		if (NULL == h) {
			return names;
		}
		libusb_close(h);

		// scan for filter wheels
		std::unique_lock<std::mutex>	lock(_hid_mutex);
		struct hid_device_info	*hinfo = hid_enumerate(SX_VENDOR_ID,
			SX_FILTERWHEEL_PRODUCT_ID);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got hid enumeration %p", hinfo);
		struct hid_device_info	*p = hinfo;
		while (p) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "got HID at %p", hinfo);
			std::string	serial("080");
			if (p->serial_number) {
				serial = wchar2string(p->serial_number);
			}
			DeviceName	name(DeviceName::Filterwheel,
					std::string(SX_MODULE_NAME), serial);
			names.push_back(name);
			p = p->next;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d filterwheels found",
			names.size());
		return names;
	}

	// list all devices from the context, ignoring filterwheel devices
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"iterating through remaining device types");
	std::vector<DevicePtr>	d = context.devices((uint16_t)SX_VENDOR_ID);
	std::vector<DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		// try to open the device. On Mac OS X, opening doesn't fail
		// ever, but on Linux, we may not have permission to open
		// all devices. We ignore devices that we cannot open
		try {
			DevicePtr	devptr = *i;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %04x:%04x",
				devptr->getVendorId(), devptr->getProductId());
			// skip filter wheels
			if (devptr->getProductId() == SX_FILTERWHEEL_PRODUCT_ID)
				continue;
			devptr->open();
			try {
				addname(&names, device, devptr);
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
	// verify that this is a camera name
	if (!name.hasType(DeviceName::Camera)) {
		std::string	msg = stringprintf("%s is not a Camera name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// this function works on the device map, so we have to lock it
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// first check the cache to see whether we already have this
	// device read
	std::string	enclosure = name.enclosurename();
	auto	j = _sxdevices.find(enclosure);
	if (j != _sxdevices.end()) {
		// we have found the device we are interested in and we
		// can just go ahead and build the camera
		debug(LOG_DEBUG, DEBUG_LOG, 0, "returning device %s from map",
			enclosure.c_str());
		j->second->open();
		return CameraPtr(new SxCamera(*this, j->second));
	}

	// check for a device with the right name
	try {
		DevicePtr	dptr = deviceForName(name);
		addname(NULL, DeviceName::Camera, dptr);
		return CameraPtr(new SxCamera(*this, dptr));
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("cannot find device %s: %s",
			name.toString().c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}

	// if we get to this point, we were not able to find the camera
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
	// verify that this is in fact a cooler name
	if (!name.hasType(DeviceName::Cooler)) {
		std::string	msg = stringprintf("%s is not a Cooler name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// protect against concurrent access
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// get the camera with the same name
	DeviceName	cameraname = SxName::cameraname(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		cameraname.toString().c_str());

	// now get the camera and retrieve the cooler from the camera
	CameraPtr	camera = this->getCamera(cameraname);
	CcdPtr	ccd = camera->getCcd(0);
	if (!ccd->hasCooler()) {
		std::string	msg = stringprintf("ccd %s has no cooler",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	return ccd->getCooler();
}

/**
 * \brief Get a CCD based on the CCD name
 *
 * \param name	name of the CCD
 */
CcdPtr	SxCameraLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieveing CCD '%s'",
		name.toString().c_str());
	// make sure we have a ccd name
	if (!name.hasType(DeviceName::Ccd)) {
		std::string	msg = stringprintf("%s is not a CCD name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// protect against concurrent access
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// get the camera with the same name
	DeviceName	cameraname = SxName::cameraname(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);

	// retrieve the CCD of this camera
	CcdPtr	ccd = camera->getCcd(0);
	return ccd;
}

/**
 * \brief Get an AO object
 *
 * \param name	name of the adaptive optics device
 */
AdaptiveOpticsPtr	SxCameraLocator::getAdaptiveOptics0(
		const DeviceName& name) {
	return AdaptiveOpticsPtr(new SxAO(name));
}

/**
 * \brief Get a guide port by name
 *
 * \param name	name of the guide port
 */
GuidePortPtr	SxCameraLocator::getGuidePort0(const DeviceName& name) {
	// make sure we have a guide port name
	if (!name.hasType(DeviceName::Guideport)) {
		std::string	msg = stringprintf("%s is not a Guideport name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// protect against concurrent access
	std::unique_lock<std::recursive_mutex>	lock(_mutex);

	// get the associated camera
	DeviceName	cameraname = SxName::cameraname(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for guider port of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);

	// retrieve the guide port of that camera
	if (!camera->hasGuidePort()) {
		std::string	msg = stringprintf("camera %s has no guide "
			"port", name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	return camera->getGuidePort();
}

/**
 * \brief Get the FilterWheel
 *
 * \param name	device name identifying the filter wheel
 */
FilterWheelPtr	SxCameraLocator::getFilterWheel0(const DeviceName& name) {
	FilterWheelPtr	filterwheel(new SxFilterWheel(name));
	return filterwheel;
}

} // namespace sx
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::sx::SxCameraLocator();
}
