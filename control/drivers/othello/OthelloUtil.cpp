/*
 * OthelloUtil.cpp -- utilities for the othello driver 
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <OthelloUtil.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace othello {

/**
 * \brief Build a name for the device
 *
 * \param deviceptr	USB device to build the name for
 */
std::string	othelloname(astro::usb::DevicePtr& deviceptr) {
	astro::usb::DeviceDescriptorPtr     descriptor
		= deviceptr->descriptor();
	if (OTHELLO_VENDOR_ID != descriptor->idVendor()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "device is not an Othello device");
		throw std::runtime_error("not an Othello device");
	}
	std::string	name;
	try {
		name = descriptor->iSerialNumber();
		if (name == std::string("")) {
			name = "000000";
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Othello device %s found",
			name.c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get serial number: %s",
			x.what());
		name = "000000";
	}
	return name;
}

/**
 * \brief get the name of the device inside the othello namespace
 *
 * \param deviceptr	USB device to bulid the name for
 */
DeviceName	othellodevname(astro::usb::DevicePtr& deviceptr) {
	astro::usb::DeviceDescriptorPtr     descriptor
		= deviceptr->descriptor();
	unsigned short	product = descriptor->idProduct();
	switch (product) {
	case OTHELLO_GUIDEPORT_ID:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"building guideport device name");
		return DeviceName(DeviceName::Guideport,
			std::string("othello"), othelloname(deviceptr));
	case OTHELLO_FOCUSER_ID:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"building focuser device name");
		return DeviceName(DeviceName::Focuser,
			std::string("othello"), othelloname(deviceptr));
	default:
		std::string	msg = stringprintf(
			"unknown othello product id '%hx", product);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

} // namespace othello
} // namespace camera
} // namespace astro
