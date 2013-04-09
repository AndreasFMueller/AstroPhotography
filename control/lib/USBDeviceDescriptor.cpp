/*
 * USBDeviceDescriptor.cpp -- abstraction for the device descriptor
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <ios>
#include <iomanip>

namespace astro {
namespace usb {

DeviceDescriptor::DeviceDescriptor(Device& device,
	libusb_device_descriptor *device_descriptor) : dev(device) {
	// copy the descriptor data
	memcpy(&d, device_descriptor, sizeof(libusb_device_descriptor));

	// manufacturer
	manufacturer = device.getStringDescriptor(d.iManufacturer);
	product = device.getStringDescriptor(d.iProduct);
	serialnumber = device.getStringDescriptor(d.iSerialNumber);
}

DeviceDescriptor::~DeviceDescriptor() {
}

static std::string	indent("D   ");

std::ostream&	operator<<(std::ostream& out, const DeviceDescriptor& devdesc) {
	out << indent << "bcdUSB:                        ";
	out << std::hex << devdesc.bcdUSB() << std::endl;
	out << indent << "bDeviceClass:                  ";
	out << std::dec << (int)devdesc.bDeviceClass() << std::endl;
	out << indent << "bDeviceSubClass:               ";
	out << (int)devdesc.bDeviceSubClass() << std::endl;
	out << indent << "bDeviceProtocol:               ";
	out << (int)devdesc.bDeviceProtocol() << std::endl;
	out << indent << "bMaxPacketSize0:               ";
	out << (int)devdesc.bMaxPacketSize0() << std::endl;
	out << indent << "idVendor:                      ";
	out << std::hex << std::setfill('0') << std::right << std::setw(4);
	out << devdesc.idVendor() << std::endl;
	out << indent << "idProduct:                     ";
	out << std::hex << std::setfill('0') << std::right << std::setw(4);
	out << devdesc.idProduct() << std::endl;
	out << indent << "bcdDevice:                     ";
	out << devdesc.bcdDevice() << std::endl;
	out << indent << "iManufacturer:                 ";
	out << devdesc.iManufacturer() << std::endl;
	out << indent << "iProduct:                      ";
	out << devdesc.iProduct() << std::endl;
	out << indent << "iSerialNumber:                 ";
	out << devdesc.iSerialNumber() << std::endl;
	out << indent << "bNumConfigurations:            ";
	out << std::dec << (int)devdesc.bNumConfigurations() << std::endl;
	return out;
}


uint16_t        DeviceDescriptor::bcdUSB() const {
	return d.bcdUSB;
}

uint8_t         DeviceDescriptor::bDeviceClass() const {
	return d.bDeviceClass;
}

uint8_t         DeviceDescriptor::bDeviceSubClass() const {
	return d.bDeviceSubClass;
}

uint8_t         DeviceDescriptor::bDeviceProtocol() const {
	return d.bDeviceProtocol;
}

uint8_t         DeviceDescriptor::bMaxPacketSize0() const {
	return d.bMaxPacketSize0;
}

uint16_t        DeviceDescriptor::idVendor() const {
	return d.idVendor;
}

uint16_t        DeviceDescriptor::idProduct() const {
	return d.idProduct;
}

uint16_t        DeviceDescriptor::bcdDevice() const {
	return d.bcdDevice;
}

const std::string&     DeviceDescriptor::iManufacturer() const {
	return manufacturer;
}

const std::string&     DeviceDescriptor::iProduct() const {
	return product;
}

const std::string&     DeviceDescriptor::iSerialNumber() const {
	return serialnumber;
}

uint8_t DeviceDescriptor::bNumConfigurations() const {
	return d.bNumConfigurations;
}

} // namespace usb
} // namespace astro
