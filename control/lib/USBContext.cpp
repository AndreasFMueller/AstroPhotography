/*
 * USBContext.cpp -- abstraction for the usb context of libusb
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>
#include <debug.h>

namespace astro {
namespace usb {

// Context implementation

/**
 * \brief Create an USB context
 *
 */
Context::Context() throw(USBError) {
	int	rc = libusb_init(&context);
	if (rc != 0) {
		throw USBError(libusb_error_name(rc));
	}
	const libusb_version	*version = libusb_get_version();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "libusb version: %hu.%hu.%hu",
		version->major, version->minor, version->micro);
}

/**
 * \brief Destroy the USB context
 */
Context::~Context() {
	libusb_exit(context);
}

/**
 * \brief set the debug level
 *
 * \param level	The debug level is identical to the libusb debug level.
 */
void	Context::setDebugLevel(int level) throw (std::range_error) {
	if ((level < 0) || (level > 3)) {
		throw std::range_error("invalid USB debug level");
	}
	libusb_set_debug(context, level);
}

/**
 * \brief Retrieve a list of devices available within this context
 */
std::vector<DevicePtr>	Context::devices() throw (USBError) {
	std::vector<DevicePtr>	result;
	libusb_device	**devlist;
	ssize_t	length = libusb_get_device_list(context, &devlist);
	if (length < 0) {
		throw USBError(libusb_error_name(length));
	}
	if (length > 0) {
		for (int i = 0; i < length; i++) {
			DevicePtr	dev(new Device(this, devlist[i]));
			result.push_back(dev);
		}
	}
	// creating Device objects increases the reference count, 
	// so it is save to unref all devices, we don't need them
	// any more
	libusb_free_device_list(devlist, 1);
	return result;
}

/**
 * \brief Open device based on vendor_id and product_id
 */
DevicePtr	Context::find(uint16_t vendor_id, uint16_t product_id)
	throw(USBError) {
	// open the device handle
	libusb_device_handle	*dev_handle = libusb_open_device_with_vid_pid(
						context, vendor_id, product_id);
	if (NULL == dev_handle) {
		throw USBError("cannot open device");
	}

	// get the device structure for thie device handle
	Device	*devptr = new Device(this, libusb_get_device(dev_handle),
		dev_handle);
	return DevicePtr(devptr);
}

/**
 * \brief get the Libusb context
 */
libusb_context	*Context::getLibusbContext() const {
	return context;
}

} // namespace usb
} // namespace astro
