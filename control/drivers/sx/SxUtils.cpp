/*
 * SxUtils.cpp -- starlight express utilities 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

SxError::SxError(const char *cause) : std::runtime_error(cause) {
}

/**
 * \brief Auxiliary function to generate the camera name from the deviceptr
 *
 * When this function is called, the device the argument deviceptr points
 * to must be open.
 */
std::string	sxname(DevicePtr& deviceptr) {
debug(LOG_DEBUG, DEBUG_LOG, 0, "sxname call: %p", &*deviceptr);
	DeviceDescriptorPtr     descriptor = deviceptr->descriptor();
	if (SX_VENDOR_ID != descriptor->idVendor()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "device is not an SX camera");
		throw std::runtime_error("not an SX device");
	}
	std::string     name = stringprintf(
		"%03d-%03d-%s-%04x-%04x",
		deviceptr->getBusNumber(),
		deviceptr->getDeviceAddress(),
		descriptor->iProduct().c_str(),
		descriptor->idVendor(),
		descriptor->idProduct());
debug(LOG_DEBUG, DEBUG_LOG, 0, "name = %s", name.c_str());
	if (descriptor->iSerialNumber().size() > 0) {
		name.append("-");
		name.append(descriptor->iSerialNumber());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SX device %s found", name.c_str());
	return name;
}

/**
 * \brief parse a device name into bus number and device address
 *
 * This method assumes that the string was formatted using the sxname
 * function. It therefore extracts substrings for busnumber and
 * device address at the position and length where sxname would but
 * these numbers, and converts them into integers.
 */
void	sxparse(const std::string& name, int& busnumber, int& deviceaddress) {
	std::string	busnumberstring = name.substr(0, 3);	
	std::string	deviceaddressstring = name.substr(4,3);
	busnumber = stoi(busnumberstring);
	deviceaddress = stoi(deviceaddressstring);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has bus=%d, addr=%d", name.c_str(),
                busnumber, deviceaddress);
}

} // namespace sx
} // namespace camera
} // namespace astro
