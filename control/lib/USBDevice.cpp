/*
 * USBDevice.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

void	Device::getDescriptor(struct libusb_device_descriptor *devdesc) const {
	libusb_get_device_descriptor(dev, devdesc);
}

void	Device::open() throw(USBError) {
	// handle the case where the device has already been opened
	if (NULL != dev_handle) {
		return;
	}

	// the device is not open yet, so open it
	int	rc = libusb_open(dev, &dev_handle);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::close() {
	if (NULL == dev_handle) {
		return;
	}
	libusb_close(dev_handle);
	dev_handle = NULL;
}

Device::Device(libusb_device *_dev, libusb_device_handle *_dev_handle)
	: dev(_dev), dev_handle(_dev_handle) {
	// increment the reference counter
	libusb_ref_device(dev);

	// find out whether this is a broken device
	DeviceDescriptorPtr	d = descriptor();
	if (d->idVendor() == 0x199e) {
std::cout << "This is a broken camera from the imaging source" << std::endl;
		broken = BROKEN_THE_IMAGING_SOURCE;
	}
}

Device::~Device() {
	close();
	libusb_unref_device(dev);
}

std::string	Device::getStringDescriptor(uint8_t index)
	const throw(USBError) {
	if (NULL == dev_handle) {
		return std::string("(device not open)");
	}
	// read the string descriptor
	unsigned char	buffer[128];
	int	rc = libusb_get_string_descriptor_ascii(dev_handle, index,
			buffer, sizeof(buffer));
	if (rc > 0) {
		return std::string((const char *)buffer, rc);
	}
	return std::string();
}

DeviceDescriptorPtr	Device::descriptor() throw(USBError) {
	// get the device descriptor
	libusb_device_descriptor	d;
	int	rc = libusb_get_device_descriptor(dev, &d);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}

	// create a DeviceDescriptor object
	DeviceDescriptor	*devdesc = new DeviceDescriptor(*this, &d);
	return DeviceDescriptorPtr(devdesc);
}

ConfigurationPtr	Device::config(uint8_t index) throw(USBError) {
	struct libusb_config_descriptor	*config = NULL;
	int	rc = libusb_get_config_descriptor(dev, index, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

ConfigurationPtr	Device::activeConfig() throw(USBError) {
	struct libusb_config_descriptor	*config = NULL;
	int	rc = libusb_get_active_config_descriptor(dev, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

ConfigurationPtr	Device::configValue(uint8_t value) throw(USBError) {
	struct libusb_config_descriptor	*config;
	int	rc = libusb_get_config_descriptor_by_value(dev, value, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	Configuration	*result = new Configuration(*this, config);
	libusb_free_config_descriptor(config);
	return ConfigurationPtr(result);
}

uint8_t	Device::getBusNumber() const {
	return libusb_get_bus_number(dev);
}

uint8_t	Device::getDeviceAddress() const {
	return libusb_get_device_address(dev);
}

int	Device::getBroken() const {
	return broken;
}

void	Device::claimInterface(uint8_t interface) throw(USBError) {
	int	rc = libusb_claim_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::releaseInterface(uint8_t interface) throw(USBError) {
	int	rc = libusb_release_interface(dev_handle, interface);
	rc = libusb_release_interface(dev_handle, interface);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

int	Device::getConfiguration() throw(USBError) {
	int	result;
	int	rc = libusb_get_configuration(dev_handle, &result);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	return result;
}

void	Device::setConfiguration(uint8_t configuration) throw (USBError) {
	int	rc = libusb_set_configuration(dev_handle, configuration);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::setInterfaceAltSetting(uint8_t interface, uint8_t altsetting)
		throw(USBError) {
	int	rc = libusb_set_interface_alt_setting(dev_handle,
			interface, altsetting);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

void	Device::controlRequest(RequestPtr request) throw(USBError) {
	std::cout << request->toString();
	int	rc = libusb_control_transfer(dev_handle, 
			request->bmRequestType(),
			request->bRequest(),
			request->wValue(),
			request->wIndex(),
			request->payload(),
			request->wLength(),
			100);
	std::cout << "rc = " << (int)rc << std::endl;
	if (rc < 0) {
		throw USBError(libusb_error_name(rc));
	}
	if (rc != request->wLength()) {
		throw USBError("request did not return what we expected");
	}
	//std::cout << request->toString();
}


enum Device::usb_speed	Device::getDeviceSpeed() const {
	return (Device::usb_speed)libusb_get_device_speed(dev);
}

std::ostream&	operator<<(std::ostream& out, const Device& device) {
	out	<< "bus " << (int)device.getBusNumber()
		<< " address " << (int)device.getDeviceAddress()
		<< " speed " << (int)device.getDeviceSpeed() << std::endl;
	return out;
}

int	Device::maxIsoPacketSize(uint8_t endpoint) const {
	return libusb_get_max_iso_packet_size(dev, endpoint);
}

void	Device::submit(TransferPtr request) throw(USBError) {
}

} // namespace usb
} // namespace astro
