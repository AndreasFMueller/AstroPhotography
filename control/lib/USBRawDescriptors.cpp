/*
 * USBRawDescriptors.cpp -- descriptors that don't have a libusb structure
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// USBDescriptor base class
//////////////////////////////////////////////////////////////////////

USBDescriptor::USBDescriptor(const Device& _device, const void *_data, int length)
	: device(_device) {
	if (length < 2) {
		throw std::length_error("data block too short for descriptor");
	}
	blength = ((uint8_t *)_data)[0];
	if (blength > length) {
		throw std::length_error("not enough data for this descriptor");
	}
	bdescriptortype = ((uint8_t *)_data)[1];
	data = new uint8_t[blength];
	memcpy(data, _data, blength);
}

USBDescriptor::USBDescriptor(const USBDescriptor& other)
	: device(other.device) {
	blength = other.blength;
	bdescriptortype = other.bdescriptortype;
	data = new uint8_t[blength];
	memcpy(data, other.data, blength);
}

USBDescriptor&	USBDescriptor::operator=(const USBDescriptor& other) {
	device = other.device;
	// copy simple files
	bdescriptortype = other.bdescriptortype;
	blength = other.blength;
	// create a copy of the data
	uint8_t	*newdata = new uint8_t[blength];
	memcpy(newdata, other.data, blength);
	delete[] data;
	data = newdata;
}

USBDescriptor::~USBDescriptor() {
	delete[] data;
}

uint8_t	USBDescriptor::bLength() const {
	return blength;
}

uint8_t	USBDescriptor::bDescriptorType() const {
	return bdescriptortype;
}

std::string	USBDescriptor::toString() const {
	std::ostringstream	out;
	out << "bLength:         " << (int)bLength() << std::endl;
	out << "bDescriptorType: " << (int)bDescriptorType() << std::endl;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const USBDescriptor& descriptor) {
	return out << descriptor.toString();
}

std::ostream&	operator<<(std::ostream& out,
		const USBDescriptorPtr& descriptorptr) {
	return out << descriptorptr->toString();
}

//////////////////////////////////////////////////////////////////////
// UnknownDescriptorError
//////////////////////////////////////////////////////////////////////
static std::string	errormessage(uint8_t length, uint8_t descriptortype) {
	char	buffer[128];
	snprintf(buffer, sizeof(buffer), "descriptor type %d, length %d",
		(int)length, (int)descriptortype);
	return std::string(buffer);
}

UnknownDescriptorError::UnknownDescriptorError(uint8_t length, uint8_t type)
	: std::runtime_error(errormessage(length, type).c_str()) {
}

//////////////////////////////////////////////////////////////////////
// Descriptor factory for "raw" descriptor
//////////////////////////////////////////////////////////////////////
DescriptorFactory::DescriptorFactory(const Device& _device) : device(_device) {
}

USBDescriptorPtr	DescriptorFactory::descriptor(const void *data, int length) 
	throw(std::length_error, UnknownDescriptorError) {
	uint8_t	blength = ((uint8_t *)data)[0];
	uint8_t	bdescriptortype = ((uint8_t *)data)[1];
	if (blength > length) {
		throw std::length_error("data to small for descriptor");
	}
	switch (bdescriptortype) {
	case 11:
		return USBDescriptorPtr(new InterfaceAssociationDescriptor(device,
			data, blength));
		break;
	}
	throw UnknownDescriptorError(blength, bdescriptortype);
}

std::list<USBDescriptorPtr>	DescriptorFactory::descriptors(const void *data, int length) throw(std::length_error, UnknownDescriptorError) {
	std::list<USBDescriptorPtr>	result;
	int	offset = 0;
	do {
		uint8_t	*a = offset + (uint8_t *)data;
		int	l = a[0];
		if (offset + l <= length) {
			try {
				result.push_back(this->descriptor(a, length - offset));
			} catch (UnknownDescriptorError& x) {

			}
		}
		offset += l;
	} while (offset < length);
	return result;
}

std::ostream&	operator<<(std::ostream& out, const std::list<USBDescriptorPtr>& list) {
	std::list<USBDescriptorPtr>::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		out << *i;
	}
	return out;
}

//////////////////////////////////////////////////////////////////////
// InterfaceAssociationDescriptor
//////////////////////////////////////////////////////////////////////

InterfaceAssociationDescriptor::InterfaceAssociationDescriptor(const Device& _device, const void *_data, int length) : USBDescriptor(_device, _data, length) {
	DeviceHandle	*handle = device.open();
	function = handle->getStringDescriptor(data[7]);
	delete handle;
}

InterfaceAssociationDescriptor::InterfaceAssociationDescriptor(const InterfaceAssociationDescriptor& other) : USBDescriptor(other) {
	function = other.function;
}

InterfaceAssociationDescriptor&	InterfaceAssociationDescriptor::operator=(const InterfaceAssociationDescriptor& other) {
	function = other.function;
	return *this;
}

uint8_t	InterfaceAssociationDescriptor::bFirstInterface() const {
	return data[2];
}

uint8_t	InterfaceAssociationDescriptor::bInterfaceCount() const {
	return data[3];
}

uint8_t	InterfaceAssociationDescriptor::bFunctionClass() const {
	return data[4];
}

uint8_t	InterfaceAssociationDescriptor::bFunctionSubClass() const {
	return data[5];
}

uint8_t	InterfaceAssociationDescriptor::bFunctionProtocol() const {
	return data[6];
}

const std::string&	InterfaceAssociationDescriptor::iFunction() const {
	return function;
}

std::string	InterfaceAssociationDescriptor::toString() const {
	std::ostringstream	out;
	out << "bFirstInterface:   ";
	out << bFirstInterface() << std::endl;
	out << "bInterfaceCount:   ";
	out << bInterfaceCount() << std::endl;
	out << "bFunctionClass:    ";
	out << bFunctionClass() << std::endl;
	out << "bFunctionSubClass: ";
	out << bFunctionSubClass() << std::endl;
	out << "bFunctionProtocol: ";
	out << bFunctionProtocol() << std::endl;
	out << "iFunction:         ";
	out << iFunction() << std::endl;
	return out.str();
}

} // namespace usb
} // namespace astro
