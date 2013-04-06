/*
 * USBDeviceHandle.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

DeviceHandle::DeviceHandle(const Device& _device, libusb_device_handle *handle)
	: dev(_device), dev_handle(handle) {
}

DeviceHandle::~DeviceHandle() {
	if (dev_handle) {
		libusb_close(dev_handle);
		dev_handle = NULL;
	}
}

Device	DeviceHandle::device() {
	return dev;
}

void	DeviceHandle::claimInterface(int interface) throw(USBError) {
	int	rc = libusb_claim_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

void	DeviceHandle::releaseInterface(int interface) throw(USBError) {
	int	rc = libusb_release_interface(dev_handle, interface);
	rc = libusb_release_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

int	DeviceHandle::getConfiguration() throw(USBError) {
	int	result;
	int	rc = libusb_get_configuration(dev_handle, &result);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	return result;
}

void	DeviceHandle::setConfiguration(int configuration) throw (USBError) {
	int	rc = libusb_set_configuration(dev_handle, configuration);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

std::string	DeviceHandle::getStringDescriptor(uint8_t index) const {
	unsigned char	buffer[128];
	int	rc = libusb_get_string_descriptor_ascii(dev_handle, index,
			buffer, sizeof(buffer));
	if (rc > 0) {
		return std::string((const char *)buffer, rc);
	}
	return std::string();
}

int	DeviceHandle::controlRequest(RequestBase *request) throw(USBError) {
	//std::cout << request->toString();
	int	rc = libusb_control_transfer(dev_handle, 
			request->bmRequestType(),
			request->bRequest(),
			request->wValue(),
			request->wIndex(),
			request->payload(),
			request->wLength(),
			100);
	//std::cout << "rc = " << (int)rc << std::endl;
	if (rc != request->wLength()) {
		throw USBError("request did not return what we expected");
	}
	if (rc < 0) {
		return rc;
	}
	//std::cout << request->toString();
	return rc;
}

} // namespace usb
} // namespace astro
