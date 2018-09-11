/*
 * OthelloLocator.cpp -- camera locator class for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OthelloLocator.h>
#include <OthelloUtil.h>
#include <OthelloGuidePort.h>
#include <OthelloFocuser.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <AstroUtils.h>
#include <includes.h>
#include <AstroExceptions.h>

using namespace astro::usb;

namespace astro {
namespace module {
namespace othello {

//////////////////////////////////////////////////////////////////////
// Implementation of the Othello Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      othello_name("othello");
static std::string      othello_version(VERSION);

/**
 * \brief Module descriptor for the Othello module
 */
class OthelloDescriptor : public ModuleDescriptor {
public:
	OthelloDescriptor() { }
	~OthelloDescriptor() { }
        virtual std::string     name() const {
                return othello_name;
        }
        virtual std::string     version() const {
                return othello_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace othello
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::othello::OthelloDescriptor();
}

namespace astro {
namespace camera {
namespace othello {

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for Othello Devices
//////////////////////////////////////////////////////////////////////

OthelloLocator::OthelloLocator() {
	// context.setDebugLevel(0);
}

OthelloLocator::~OthelloLocator() {
}

/**
 * \brief Get module name.
 */
std::string	OthelloLocator::getName() const {
	return astro::module::othello::othello_name;
}

/**
 * \brief Get module version.
 */
std::string	OthelloLocator::getVersion() const {
	return VERSION;
}

/**
 * \brief Get a list of Othello devices
 *
 * \param device	type of devices to list
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	OthelloLocator::getDevicelist(DeviceName::device_type device) {
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
				DeviceDescriptorPtr	descriptor
					= devptr->descriptor();
				if (descriptor->idVendor()
					!= OTHELLO_VENDOR_ID) continue;
				// handle guider port devices
				if ((descriptor->idProduct()
					== OTHELLO_GUIDEPORT_ID)
					&& (device == DeviceName::Guideport)) {
					names.push_back("guideport:othello/"
						+ othelloname(devptr));
				}
				// handle focuser devices
				if ((descriptor->idProduct()
					== OTHELLO_FOCUSER_ID)
					&& (device == DeviceName::Focuser)) {
					names.push_back("focuser:othello/"
						+ othelloname(devptr));
				}
			} catch (std::runtime_error& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "found a non"
					" Othello device: %s", x.what());
			}
			devptr->close();
		} catch (std::exception& x) {
			// log the error, but don't do anything about it
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot work with device: "
				"'%s', skipping", x.what());
		} catch (...) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot work with device, "
				"skipping");
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d othello devices",
		names.size());
	return names;
}

/**
 * \brief Get a guider port by name
 */
GuidePortPtr	OthelloLocator::getGuidePort0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for device %s",
		name.toString().c_str());
	// extract the serial number from the name
	std::string	serial = name.unitname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for device unit %s",
		serial.c_str());

	// find the device with this serial number
	std::vector<astro::usb::DevicePtr>	d = context.devices();
	std::vector<astro::usb::DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		try {
			astro::usb::DevicePtr	dptr = (*i);
			DeviceDescriptorPtr	descriptor = dptr->descriptor();
			uint16_t	vendor = descriptor->idVendor();
			if (vendor != OTHELLO_VENDOR_ID) {
				continue;
			}
			if (descriptor->idProduct() != OTHELLO_GUIDEPORT_ID) {
				continue;
			}
			bool	needsclosing = true;
			if (dptr->isOpen()) {
				needsclosing = false;
			} else {
				dptr->open();
			}
			std::string	devserial
				= dptr->descriptor()->iSerialNumber();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "device serial: %s",
				devserial.c_str());
			if (devserial == serial) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"matching guider port");
				return GuidePortPtr(new OthelloGuidePort(dptr));
			}
			if (needsclosing) {
				dptr->close();
			}
		} catch (const std::runtime_error& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot work with "
				"device: '%s', skipping", x.what());
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot work with "
				"device, skipping");
		}
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "coult no find device %s",
		name.toString().c_str());
	throw std::runtime_error("device not found");
}

/**
 * \brief Create a focuser from the name
 *
 * \param name	name of the focuser
 */
FocuserPtr	OthelloLocator::getFocuser0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for device %s",
		name.toString().c_str());
	// extract the serial number from the name
	std::string	serial = name.unitname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for device with serial %s",
		serial.c_str());

	// find the device with this serial number
	std::vector<astro::usb::DevicePtr>	d = context.devices();
	std::vector<astro::usb::DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		try {
			astro::usb::DevicePtr	dptr = (*i);
			DeviceDescriptorPtr	descriptor = dptr->descriptor();
			uint16_t	vendor = descriptor->idVendor();
			uint16_t	product = descriptor->idProduct();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %hx:%hx",
				vendor, product);
			if (vendor != OTHELLO_VENDOR_ID) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"wrong vendor %hx", vendor);
				continue;
			}
			if (product != OTHELLO_FOCUSER_ID) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"wrong product %hx", product);
				continue;
			}
			bool	needsclosing = true;
			if (dptr->isOpen()) {
				needsclosing = false;
			} else {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"opening device");
				dptr->open();
				// reread the descriptor, as the serial number
				// was not accessible during when the device
				// was not open
				descriptor = dptr->descriptor();
			}
			std::string	devserial = descriptor->iSerialNumber();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "device serial: %s",
				devserial.c_str());
			if (devserial == serial) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"matching focuser");
				return FocuserPtr(new OthelloFocuser(dptr));
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"device serial %s does not match",
				serial.c_str());
			if (needsclosing) {
				dptr->close();
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot work with "
				"device: '%s', skipping", x.what());
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot work with "
				"device, skipping");
		}
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "coult no find device %s",
		name.toString().c_str());
	throw std::runtime_error("device not found");
}

} // namespace othello
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::othello::OthelloLocator();
}
