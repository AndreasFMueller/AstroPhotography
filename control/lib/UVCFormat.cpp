/*
 * UVCFormat.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <sstream>

using namespace astro::usb;
using namespace astro::usb::uvc;

namespace astro {
namespace usb {
namespace uvc {

//////////////////////////////////////////////////////////////////////
// Video Streaming Interface
//////////////////////////////////////////////////////////////////////

VideoStreamingInterface::VideoStreamingInterface(const Interface& _interface) {
	interface = _interface.interfaceNumber();
}

//////////////////////////////////////////////////////////////////////
// Video Streaming Interface Descriptor
//////////////////////////////////////////////////////////////////////

VideoStreamingInterfaceDescriptor::VideoStreamingInterfaceDescriptor(
	const InterfaceDescriptor& interfacedescriptor) {
	altsetting = interfacedescriptor.bAlternateSetting();
}

//////////////////////////////////////////////////////////////////////
// Format factory
//////////////////////////////////////////////////////////////////////

bool	isFormatDescriptor(USBDescriptorPtr dp) {
	FormatDescriptor	*fdp = dynamic_cast<FormatDescriptor *>(&*dp);
	return (fdp != NULL) ? true : false;
}

FormatDescriptorFactory::FormatDescriptorFactory(const Device& _device)
	: UVCDescriptorFactory(_device) {
}

FormatDescriptorPtr	FormatDescriptorFactory::formatdescriptor(
	const void *data, int length)
	throw(std::length_error, UnknownDescriptorError) {
	FormatDescriptorPtr	fdp;
	return fdp;
}

//////////////////////////////////////////////////////////////////////
// FormatDescriptor
//////////////////////////////////////////////////////////////////////
FormatDescriptor::FormatDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
}

FormatDescriptor::FormatDescriptor(const FormatDescriptor& other)
	: UVCDescriptor(other) {
}

FormatDescriptor&	FormatDescriptor::operator=(
	const FormatDescriptor& other) {
	UVCDescriptor::operator=(other);
}

uint8_t	FormatDescriptor::bFormatIndex() const {
	return uint8At(3);
}

uint8_t	FormatDescriptor::bNumFrameDescriptors() const {
	return uint8At(4);
}

std::string	FormatDescriptor::toString() const {
	std::ostringstream	out;
	out << "  bFormatIndex:         ";
	out << (int)this->bFormatIndex() << std::endl;
	out << "  bNumFrameDescriptors: ";
	out << (int)this->bNumFrameDescriptors() << std::endl;
	out << "  bDefaultFrameIndex:   ";
	out << (int)this->bDefaultFrameIndex() << std::endl;
	out << "  bAspectRatioX:        ";
	out << (int)this->bAspectRatioX() << std::endl;
	out << "  bAspectRatioY:        ";
	out << (int)this->bAspectRatioY() << std::endl;
	out << "  bmInterlaceFlags:     ";
	out << std::hex << (int)this->bmInterlaceFlags() << std::endl;
	out << "  bCopyProtect:         ";
	out << (int)this->bCopyProtect() << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// FrameDescriptor
//////////////////////////////////////////////////////////////////////
FrameDescriptor::FrameDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
}

FrameDescriptor::FrameDescriptor(const FrameDescriptor& other)
	: UVCDescriptor(other) {
}

FrameDescriptor&	FrameDescriptor::operator=(
	const FrameDescriptor& other) {
	UVCDescriptor::operator=(other);
}

uint8_t	FrameDescriptor::bFrameIndex() const {
	return uint8At(3);
}

uint32_t	FrameDescriptor::bmCapabilities() const {
	return bitmapAt(4, 1);
}

uint16_t	FrameDescriptor::wWidth() const {
	return uint16At(5);
}

uint16_t	FrameDescriptor::wHeight() const {
	return uint16At(7);
}

uint32_t	FrameDescriptor::dwMinBitRate() const {
	return uint32At(9);
}

uint32_t	FrameDescriptor::dwMaxBitRate() const {
	return uint32At(13);
}

uint32_t	FrameDescriptor::dwDefaultFrameInterval() const {
	return uint32At(21);
}

uint8_t	FrameDescriptor::bFrameIntervalType() const {
	return uint8At(25);
}

uint32_t	FrameDescriptor::dwMinFrameInterval() const {
	if (0 != bFrameIntervalType()) {
		throw std::range_error("discrete frame interval");
	}
	return uint32At(26);
}

uint32_t	FrameDescriptor::dwMaxFrameInterval() const {
	if (0 != bFrameIntervalType()) {
		throw std::range_error("discrete frame interval");
	}
	return uint32At(30);
}

uint32_t	FrameDescriptor::dwFrameIntervalStep() const {
	if (0 != bFrameIntervalType()) {
		throw std::range_error("discrete frame interval");
	}
	return uint32At(34);
}

uint32_t	FrameDescriptor::dwFrameInterval(int index) const {
	if ((index < 0) || (index >= bFrameIntervalType())) {
		std::range_error("out of frame interval range");
	}
	return uint32At(26 + 4 * index);
}

std::string	FrameDescriptor::toString() const {
	std::ostringstream	out;
	out << "  bFrameIndex:               ";
	out << (int)this->bFrameIndex() << std::endl;
	out << "  bmCapabilities:            ";
	out << std::hex << this->bmCapabilities() << std::endl;
	out << "  wWidth:                    ";
	out << std::dec << this->wWidth() << std::endl;
	out << "  wHeight:                   ";
	out << std::dec << this->wHeight() << std::endl;
	out << "  dwMinBitRate:              ";
	out << this->dwMinBitRate() << std::endl;
	out << "  dwMaxBitRate:              ";
	out << this->dwMaxBitRate() << std::endl;
	out << "  dwDefaultFrameInterval:    ";
	out << this->dwDefaultFrameInterval() << std::endl;
	out << "  bFrameIntervalType:        ";
	out << ((this->bFrameIntervalType()) ? "discrete" : "continuous")
		<< std::endl;
	if (0 == bFrameIntervalType()) {
		out << "  dwMinFrameInterval:        ";
		out << this->dwMinFrameInterval() << std::endl;
		out << "  dwMaxFrameInterval:        ";
		out << this->dwMaxFrameInterval() << std::endl;
		out << "  dwFrameIntervalStep:       ";
		out << this->dwFrameIntervalStep() << std::endl;
	} else {
		for (int index = 0; index < this->bFrameIntervalType(); index++) {
			out << "  dWFrameInterval(" << index << ") = ";	
			out << this->dwFrameInterval(index) << std::endl;
		}
	}
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
