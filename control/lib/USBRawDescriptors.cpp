/*
 * USBRawDescriptors.cpp -- descriptors that don't have a libusb structure
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <AstroDebug.h>

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// USBDescriptor base class
//////////////////////////////////////////////////////////////////////

USBDescriptor::USBDescriptor(Device& _device, const void *_data, int length)
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

uint8_t USBDescriptor::uint8At(int offset) const {
	return ((uint8_t *)data)[offset];
}

int8_t  USBDescriptor::int8At(int offset) const {
	return (int8_t)uint8At(offset);
}

uint16_t        USBDescriptor::uint16At(int offset) const {
	uint16_t	result = uint8At(offset + 1);
	result = (result << 8) | uint8At(offset);
	return result;
}

int16_t USBDescriptor::int16At(int offset) const {
	return (int16_t)uint16At(offset);
}

uint32_t        USBDescriptor::uint32At(int offset) const {
	uint32_t	result = uint8At(offset + 3);
	result = (result << 8) | uint8At(offset + 2);
	result = (result << 8) | uint8At(offset + 1);
	result = (result << 8) | uint8At(offset);
	return result;
}

int32_t USBDescriptor::int32At(int offset) const {
	return (int32_t)uint32At(offset);
}

uint32_t        USBDescriptor::bitmapAt(int offset, int size) const {
	uint32_t	mask = 0xff;
	for (int i = 2; i <= size; i++) {
		mask |= mask << 8;
	}
	return mask & uint32At(offset);
}

int	USBDescriptor::descriptorLength() const {
	return blength;
}

//////////////////////////////////////////////////////////////////////
// UnknownDescriptorError
//////////////////////////////////////////////////////////////////////
static std::string	errormessage(uint8_t length, uint8_t descriptortype) {
	char	buffer[128];
	snprintf(buffer, sizeof(buffer), "descriptor type %d, length %d",
		(int)descriptortype, (int)length);
	return std::string(buffer);
}

static std::string	errormessage(uint8_t length, uint8_t descriptortype, uint8_t descriptorsubtype) {
	char	buffer[128];
	snprintf(buffer, sizeof(buffer), "descriptor type %d/%d, length %d",
		(int)descriptortype, (int)descriptorsubtype, (int)length);
	return std::string(buffer);
}

UnknownDescriptorError::UnknownDescriptorError(uint8_t length, uint8_t type)
	: std::runtime_error(errormessage(length, type).c_str()) {
}

UnknownDescriptorError::UnknownDescriptorError(uint8_t length, uint8_t type,
	uint8_t	subtype)
	: std::runtime_error(errormessage(length, type, subtype).c_str()) {
}

//////////////////////////////////////////////////////////////////////
// Descriptor factory for "raw" descriptor
//////////////////////////////////////////////////////////////////////
DescriptorFactory::DescriptorFactory(Device& _device) : device(_device) {
}

uint8_t	DescriptorFactory::blength(const void *data) throw(std::length_error) {
	uint8_t	b = ((uint8_t *)data)[0];
	if (b < 3) {
		throw std::length_error("not enough data for descriptor");
	}
	return b;
}

uint8_t	DescriptorFactory::bdescriptortype(const void *data) {
	return ((uint8_t *)data)[1];
}

USBDescriptorPtr	DescriptorFactory::descriptor(const void *data,
	int length)
	throw(std::length_error, UnknownDescriptorError) {
	if (blength(data) > length) {
		throw std::length_error("data to small for descriptor");
	}
	USBDescriptorPtr	dp;
	switch (bdescriptortype(data)) {
	case 11:
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"create an InterfaceAssociationDescriptor");
		dp = USBDescriptorPtr(
			new InterfaceAssociationDescriptor(device,
				data, blength(data)));
		break;
	default:
		debug(LOG_ERR, DEBUG_LOG, 0,
			"trying to build unknown descriptor");
		throw UnknownDescriptorError(blength(data),
			bdescriptortype(data));
	}
	return dp;
}

USBDescriptorPtr	DescriptorFactory::descriptor(const std::string& data)
	throw(std::length_error, UnknownDescriptorError) {
	return this->descriptor(data.c_str(), data.size());
}

/**
 * \brief Get a set of descriptors from a block of data.
 *
 * The USB video class uses many class specific descriptors, which
 * libusb returns just as a block of raw data. This method
 * returns a vector of all descriptors that can be found within
 * the data block.
 * \param data		raw data block
 * \param length	length of the data block
 * \return A vector of descriptors parsed from the data block
 */
std::vector<USBDescriptorPtr>	DescriptorFactory::descriptors(
	const void *data, int length)
	throw(std::length_error, UnknownDescriptorError) {
	std::vector<USBDescriptorPtr>	result;
	int	offset = 0;
	do {
		uint8_t	*a = offset + (uint8_t *)data;
		int	l = a[0];
		if (offset + l <= length) {
			try {
				result.push_back(this->descriptor(a,
					length - offset));
			} catch (UnknownDescriptorError& x) {
				debug(LOG_ERR, DEBUG_LOG, 0,
					"unknown descriptor: %s",x.what());
				throw x;
			}
		}
		offset += l;
	} while (offset < length);
	return result;
}

/**
 * \brief Get a set of descriptors a string.
 *
 * The data to build the descriptors can also been given in the form of
 * a string.
 * \param data	extra data to parse.
 */
std::vector<USBDescriptorPtr>	DescriptorFactory::descriptors(
		const std::string& data)
		throw(std::length_error, UnknownDescriptorError) {
	if (data.size() == 0) {
		return std::vector<USBDescriptorPtr>();
	}
	return this->descriptors(data.c_str(), data.size());
}

std::ostream&	operator<<(std::ostream& out, const std::vector<USBDescriptorPtr>& list) {
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		out << *i;
	}
	return out;
}

//////////////////////////////////////////////////////////////////////
// InterfaceAssociationDescriptor
//////////////////////////////////////////////////////////////////////

InterfaceAssociationDescriptor::InterfaceAssociationDescriptor(Device& _device,
	const void *_data, int length) : USBDescriptor(_device, _data, length) {
	function = device.getStringDescriptor(data[7]);
}

uint8_t	InterfaceAssociationDescriptor::bFirstInterface() const {
	return uint8At(2);
}

uint8_t	InterfaceAssociationDescriptor::bInterfaceCount() const {
	return uint8At(3);
}

uint8_t	InterfaceAssociationDescriptor::bFunctionClass() const {
	if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
		return CC_VIDEO;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bFunctionClass = %02x", uint8At(4));
	return uint8At(4);
}

uint8_t	InterfaceAssociationDescriptor::bFunctionSubClass() const {
	if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
		return SC_VIDEO_INTERFACE_COLLECTION;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bFunctionSubClass = %02x", uint8At(5));
	return uint8At(5);
}

uint8_t	InterfaceAssociationDescriptor::bFunctionProtocol() const {
	if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
		return PC_PROTOCOL_UNDEFINED;
	}
	return uint8At(6);
}

const std::string&	InterfaceAssociationDescriptor::iFunction() const {
	return function;
}

static std::string	indent("    IAD ");

std::string	InterfaceAssociationDescriptor::toString() const {
	std::ostringstream	out;
	out << indent << "Interface Association Descriptor:" << std::endl;
	out << indent << "bFirstInterface:   ";
	out << (int)bFirstInterface() << std::endl;
	out << indent << "bInterfaceCount:   ";
	out << (int)bInterfaceCount() << std::endl;
	out << indent << "bFunctionClass:    ";
	out << (int)bFunctionClass() << std::endl;
	out << indent << "bFunctionSubClass: ";
	out << (int)bFunctionSubClass() << std::endl;
	out << indent << "bFunctionProtocol: ";
	out << (int)bFunctionProtocol() << std::endl;
	out << indent << "iFunction:         ";
	out << iFunction() << std::endl;
	return out.str();
}

bool	InterfaceAssociationDescriptor::isVideoInterfaceCollection() const {
	return ((bFunctionClass() == CC_VIDEO) &&
		(bFunctionSubClass() == SC_VIDEO_INTERFACE_COLLECTION) &&
		(bFunctionProtocol() == PC_PROTOCOL_UNDEFINED)) ? true : false;
}

} // namespace usb
} // namespace astro
