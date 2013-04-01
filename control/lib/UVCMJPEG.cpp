/*
 * UVCMJPEG.cpp
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
// FormatMJPEGDescriptor
//////////////////////////////////////////////////////////////////////
FormatMJPEGDescriptor::FormatMJPEGDescriptor(
	const Device& _device, const void *data, int length)
	: FormatDescriptor(_device, data, length) {
}

FormatMJPEGDescriptor::FormatMJPEGDescriptor(
	const FormatMJPEGDescriptor& other)
	: FormatDescriptor(other) {
}

FormatMJPEGDescriptor&	FormatMJPEGDescriptor::operator=(const FormatMJPEGDescriptor& other) {
	FormatDescriptor::operator=(other);
}

uint8_t	FormatMJPEGDescriptor::bDefaultFrameIndex() const {
	return uint8At(6);
}

uint8_t	FormatMJPEGDescriptor::bAspectRatioX() const {
	return uint8At(7);
}

uint8_t	FormatMJPEGDescriptor::bAspectRatioY() const {
	return uint8At(8);
}

uint32_t	FormatMJPEGDescriptor::bmInterlaceFlags() const {
	return bitmapAt(9, 1);
}

uint8_t	FormatMJPEGDescriptor::bCopyProtect() const {
	return uint8At(10);
}


std::string	FormatMJPEGDescriptor::toString() const {
	std::ostringstream	out;
	out << "Format MJPEG Descriptor:" << std::endl;
	out << this->FormatDescriptor::toString();
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// FrameMJPEGDescriptor
//////////////////////////////////////////////////////////////////////
FrameMJPEGDescriptor::FrameMJPEGDescriptor(const Device& _device, const void *data, int length)
	: FrameDescriptor(_device, data, length) {
}

FrameMJPEGDescriptor::FrameMJPEGDescriptor(const FrameMJPEGDescriptor& other)
	: FrameDescriptor(other) {
}

FrameMJPEGDescriptor&	FrameMJPEGDescriptor::operator=(const FrameMJPEGDescriptor& other) {
	FrameDescriptor::operator=(other);
}

uint32_t	FrameMJPEGDescriptor::dwMaxVideoFrameBufferSize() const {
	return uint32At(17);
}

std::string	FrameMJPEGDescriptor::toString() const {
	std::ostringstream	out;
	out << "Frame MJPEG Descriptor:" << std::endl;
	out << this->FrameDescriptor::toString();
	out << "  dwMaxVideoFrameBufferSize: ";
	out << dwMaxVideoFrameBufferSize() << std::endl;
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
