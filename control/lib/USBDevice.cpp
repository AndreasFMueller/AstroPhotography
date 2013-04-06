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

DeviceHandle	*Device::open() throw(USBError) {
	libusb_device_handle	*handle;
	int	rc = libusb_open(dev, &handle);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	return new DeviceHandle(*this, handle);
}

DeviceDescriptor	*Device::descriptor() const throw(USBError) {
	return new DeviceDescriptor(*this);
}

ConfigDescriptor	*Device::config(uint8_t index) const throw(USBError) {
	struct libusb_config_descriptor	*config;
	int	rc = libusb_get_config_descriptor(dev, index, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	ConfigDescriptor	*result = new ConfigDescriptor(*this, config);
	libusb_free_config_descriptor(config);
	return result;
}

ConfigDescriptor	*Device::activeConfig() const throw(USBError) {
	struct libusb_config_descriptor	*config;
	int	rc = libusb_get_active_config_descriptor(dev, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	ConfigDescriptor	*result = new ConfigDescriptor(*this, config);
	libusb_free_config_descriptor(config);
	return result;
}

ConfigDescriptor	*Device::configValue(uint8_t value) const throw(USBError) {
	struct libusb_config_descriptor	*config;
	int	rc = libusb_get_config_descriptor_by_value(dev, value, &config);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	ConfigDescriptor	*result = new ConfigDescriptor(*this, config);
	libusb_free_config_descriptor(config);
	return result;
}

Device::Device(struct libusb_device *_dev) : dev(_dev) {
	libusb_ref_device(dev);
	// find out whether this is a broken device
	DeviceDescriptor	*d = descriptor();
	if (d->idVendor() == 0x199e) {
std::cout << "This is a broken camera from the imaging source" << std::endl;
		broken = BROKEN_THE_IMAGING_SOURCE;
	}
	delete d;
}

Device::~Device() {
	libusb_unref_device(dev);
}

Device::Device(const Device& other) {
	dev = other.dev;
	broken = other.broken;
	libusb_ref_device(dev);
}

Device&	Device::operator=(const Device& other) {
	libusb_ref_device(other.dev);
	libusb_unref_device(dev);
	dev = other.dev;
	broken = other.broken;
	return *this;
}

uint8_t	Device::getBusNumber() const {
	return libusb_get_bus_number(dev);
}

uint8_t	Device::getDeviceAddress() const {
	return libusb_get_device_address(dev);
}

int	Device::getDeviceSpeed() const {
	return libusb_get_device_speed(dev);
}

int	Device::getBroken() const {
	return broken;
}

enum Device::usb_speed	Device::speed() const {
	return (Device::usb_speed)libusb_get_device_speed(dev);
}

std::ostream&	operator<<(std::ostream& out, const Device& device) {
	out	<< "bus " << (int)device.getBusNumber()
		<< " address " << (int)device.getDeviceAddress()
		<< " speed " << (int)device.getDeviceSpeed();
	return out;
}

} // namespace usb
} // namespace astro
