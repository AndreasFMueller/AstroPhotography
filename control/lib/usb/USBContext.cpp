/*
 * USBContext.cpp -- abstraction for the usb context of libusb
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <USBDebug.h>

namespace astro {
namespace usb {

// Context implementation

/**
 * \brief Create an USB context
 *
 * We keep a reference to the context so that we can be sure that the context
 * is closed only when all other USB structures have been deallocated.
 */
Context::Context() {
	context = ContextHolderPtr(new ContextHolder());
}

/**
 * \brief Destroy the USB context
 */
Context::~Context() {
}

/**
 * \brief set the debug level
 *
 * \param level	The debug level is identical to the libusb debug level.
 */
void	Context::setDebugLevel(int level) {
	if ((level < 0) || (level > 4)) {
		throw std::range_error("invalid USB debug level");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting USB debug level to %d",
		level);
	context->debug(level);
}

/**
 * \brief Retrieve a list of devices available within this context
 */
std::vector<DevicePtr>	Context::devices() {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "getting a list of all USB devices");
	std::vector<DevicePtr>	result;
	libusb_device	**devlist;
	ssize_t	length = libusb_get_device_list(context->context(), &devlist);
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "found %d devices", length);
	if (length < 0) {
		std::string	msg = stringprintf("no devices found: %s",
			libusb_error_name(length));
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}
	if (length > 0) {
		for (int i = 0; i < length; i++) {
			USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "add device %d", i);
			try {
				DevicePtr	dev(new Device(context,
							devlist[i]));
				result.push_back(dev);
			} catch (const std::exception& x) {
				USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
					"cannot add device: '%s', skipping",
					x.what());
			}
		}
	}
	// creating Device objects increases the reference count, 
	// so it is save to unref all devices, we don't need them
	// any more
	libusb_free_device_list(devlist, 1);
	return result;
}

/**
 * \brief Retrieve a list of devices available within this context
 *
 * \param vendor_id	list devices from this vendor
 */
std::vector<DevicePtr>	Context::devices(uint16_t vendor_id) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "getting devices for vendor %04hx",
		vendor_id);
	std::vector<DevicePtr>	result;

	// construct a list of devices
	libusb_device	**devlist;
	ssize_t	length = libusb_get_device_list(context->context(), &devlist);
	if (length < 0) {
		std::string	msg = stringprintf("cannot get device list: %s",
			libusb_error_name(length));
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw USBError(msg);
	}

	// scan the device list for devices from a given vendor
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "scan %d devices", length);
	for (int i = 0; i < length; i++) {
		int	rc;
		libusb_device_descriptor	desc;
		rc = libusb_get_device_descriptor(devlist[i], &desc);
		if ((rc == LIBUSB_SUCCESS) && (desc.idVendor == vendor_id)) {
			USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "device %04hx:%04hx",
				desc.idVendor, desc.idProduct);
			DevicePtr	dev(new Device(context, devlist[i]));
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
 *
 * \param vendor_id	vendor id to search for
 * \param product_id	product id to search for
 */
DevicePtr	Context::find(uint16_t vendor_id, uint16_t product_id) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "find device VID=%04hx/PID=%04hx",
		vendor_id, product_id);
	libusb_device	**devlist;
	ssize_t	length = libusb_get_device_list(context->context(), &devlist);
	if (length < 0) {
		throw USBError(libusb_error_name(length));
	}
	DevicePtr	result;
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "scan %d devices", length);
	for (int i = 0; (!result) && (i < length); i++) {
		int	rc;
		libusb_device_descriptor	desc;
		rc = libusb_get_device_descriptor(devlist[i], &desc);
		if ((rc == LIBUSB_SUCCESS) && (desc.idVendor == vendor_id)
			&& (desc.idProduct == product_id)) {
			result = DevicePtr(new Device(context, devlist[i]));
		}
	}
	libusb_free_device_list(devlist, 1);

	// get the device structure for thie device handle
	return result;
}

/**
 * \brief get the Libusb context
 */
libusb_context	*Context::getLibusbContext() const {
	return context->context();
}

} // namespace usb
} // namespace astro
