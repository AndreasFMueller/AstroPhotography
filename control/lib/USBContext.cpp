/*
 * USBContext.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

// Context implementation

/**
 * \brief Create an USB context
 */
Context::Context() throw(USBError) {
	int	rc = libusb_init(&context);
	if (rc != 0) {
		throw USBError(libusb_error_name(rc));
	}
}

/**
 * \brief Destroy the USB context
 */
Context::~Context() {
	libusb_exit(context);
}

/**
 * \brief set the debug level
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
std::list<Device>	Context::list() throw (USBError) {
	std::list<Device>	result;
	libusb_device	**devlist;
	size_t	length = libusb_get_device_list(context, &devlist);
	if (length < 0) {
		throw USBError(libusb_error_name(length));
	}
	if (length > 0) {
		for (int i = 0; i < length; i++) {
			Device	dev(devlist[i]);
			result.push_back(dev);
		}
	}
	libusb_free_device_list(devlist, 1);
	return result;
}

/**
 * \brief Open device based on vendor_id and product_id
 */
DeviceHandle	*Context::open(uint16_t vendor_id, uint16_t product_id) throw(USBError) {
	// open the device handle
	libusb_device_handle	*dev_handle = libusb_open_device_with_vid_pid(
						context, vendor_id, product_id);
	if (NULL == dev_handle) {
		throw USBError("cannot open device");
	}

	// get the device structure for thie device handle
	Device	device(libusb_get_device(dev_handle));
	DeviceHandle	*result = new DeviceHandle(device, dev_handle);
	return result;
}


} // namespace usb
} // namespace astro
