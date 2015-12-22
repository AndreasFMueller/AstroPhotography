/*
 * QhyUtils.cpp -- utility functions for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace qhy {

/**
 * \brief Construct the device name from the USB device ptr
 */
std::string	qhyname(DevicePtr deviceptr) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "qhyname call: %p", &*deviceptr);
	usb::DeviceDescriptorPtr	descriptor = deviceptr->descriptor();
	if (QHY_VENDOR_ID != descriptor->idVendor()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "device is not a QHY device");
		throw std::runtime_error("not an QHY device");
	}
	std::string	iprod = trim(descriptor->iProduct());
	std::string	name = stringprintf(
		"%03d-%03d-%s-%04x-%04x",
		deviceptr->getBusNumber(),
		deviceptr->getDeviceAddress(),
		iprod.c_str(),
		descriptor->idVendor(),
		descriptor->idProduct());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "name = %s", name.c_str());
	if (descriptor->iSerialNumber().size() > 0) {
		name.append("-");
		name.append(descriptor->iSerialNumber());
	}
	return name;
}

/**
 * \brief Parse the name into libusb bus number and device address
 */
void	qhyparse(const std::string& name, int& busnumber, int& deviceaddress) {
	std::string	busnumberstring = name.substr(0, 3);
	std::string	deviceaddressstring = name.substr(4,3);
	busnumber = stoi(busnumberstring);
	deviceaddress = stoi(deviceaddressstring);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has bus=%d, addr=%d", name.c_str(),
		busnumber, deviceaddress);
}

} // namespace qhy
} // namespace camera
} // namespace astro
