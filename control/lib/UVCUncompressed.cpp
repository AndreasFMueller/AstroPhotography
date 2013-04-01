/*
 * UVCUncompressed.cpp
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
// FormatUncompressedDescriptor
//////////////////////////////////////////////////////////////////////
FormatUncompressedDescriptor::FormatUncompressedDescriptor(
	const Device& _device, const void *data, int length)
	: FormatFrameBasedDescriptor(_device, data, length) {
}

FormatUncompressedDescriptor::FormatUncompressedDescriptor(
	const FormatUncompressedDescriptor& other)
	: FormatFrameBasedDescriptor(other) {
}

FormatUncompressedDescriptor&	FormatUncompressedDescriptor::operator=(
	const FormatUncompressedDescriptor& other) {
	FormatFrameBasedDescriptor::operator=(other);
}

std::string	FormatUncompressedDescriptor::toString() const {
	std::ostringstream	out;
	out << "Format Uncompressed Descriptor:" << std::endl;
	out << this->FormatFrameBasedDescriptor::toString();
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// FrameUncompressedDescriptor
//////////////////////////////////////////////////////////////////////
FrameUncompressedDescriptor::FrameUncompressedDescriptor(const Device& _device,
	const void *data, int length)
	: FrameDescriptor(_device, data, length) {
}

FrameUncompressedDescriptor::FrameUncompressedDescriptor(
	const FrameUncompressedDescriptor& other)
	: FrameDescriptor(other) {
}

FrameUncompressedDescriptor&	FrameUncompressedDescriptor::operator=(
	const FrameUncompressedDescriptor& other) {
	FrameDescriptor::operator=(other);
}

uint32_t	FrameUncompressedDescriptor::dwMaxVideoFrameBufferSize() const {
	return uint32At(17);
}

std::string	FrameUncompressedDescriptor::toString() const {
	std::ostringstream	out;
	out << "Frame Uncompressed Descriptor:" << std::endl;
	out << this->FrameDescriptor::toString();
	out << "  dwMaxVideoFrameBufferSize: ";
	out << dwMaxVideoFrameBufferSize() << std::endl;
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
