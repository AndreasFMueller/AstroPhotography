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
 */
std::string	othelloname(astro::usb::DevicePtr& deviceptr) {
	astro::usb::DeviceDescriptorPtr     descriptor
		= deviceptr->descriptor();
	if (OTHELLO_VENDOR_ID != descriptor->idVendor()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "device is not an Othello device");
		throw std::runtime_error("not an Othello device");
	}
	std::string	name = descriptor->iSerialNumber();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Othello device %s found", name.c_str());
	return name;
}

DeviceName	othellodevname(astro::usb::DevicePtr& deviceptr) {
	astro::usb::DeviceDescriptorPtr     descriptor
		= deviceptr->descriptor();
	unsigned short	product = descriptor->idProduct();
	switch (product) {
	case OTHELLO_GUIDERPORT_ID:
		return DeviceName(DeviceName::Guiderport,
			std::string("othello"), othelloname(deviceptr));
	case OTHELLO_FOCUSER_ID:
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
