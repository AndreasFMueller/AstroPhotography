/*
 * OthelloLocator.cpp -- camera locator class for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OthelloLocator.h>
#include <OthelloUtil.h>
#include <OthelloGuiderPort.h>
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
// Implementation of the Starlight Express Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      othello_name("othello");
static std::string      othello_version(VERSION);

/**
 * \brief Module descriptor for the Starlight express module
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
// Implementation of the Camera Locator for Starlight Express
//////////////////////////////////////////////////////////////////////

OthelloLocator::OthelloLocator() {
	context.setDebugLevel(3);
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
 * \brief Get a list of Starlight Express cameras.
 *
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
				if (device == DeviceName::Guiderport) {
					names.push_back("guiderport:othello/"
						+ othelloname(devptr));
				}
			} catch (std::runtime_error& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "found a non"
					" Othello device: %s", x.what());
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
 * \brief Get a guider port by name
 */
GuiderPortPtr	OthelloLocator::getGuiderPort0(const DeviceName& name) {
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
		astro::usb::DevicePtr	dptr = (*i);
		uint16_t	vendor = dptr->descriptor()->idVendor();
		if (vendor != OTHELLO_VENDOR_ID) {
			break;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "device vendor: %04x", vendor);
		bool	needsclosing = true;
		if (dptr->isOpen()) {
			needsclosing = false;
		} else {
			dptr->open();
		}
		std::string	devserial = dptr->descriptor()->iSerialNumber();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "device serial: %s",
			devserial.c_str());
		if (devserial == serial) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "matching guider port");
			return GuiderPortPtr(new OthelloGuiderPort(dptr));
		}
		if (needsclosing) {
			dptr->close();
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
