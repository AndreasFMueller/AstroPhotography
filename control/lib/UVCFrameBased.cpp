/*
 * UVCFrameBased.cpp
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
// FormatFrameBasedDescriptor
//////////////////////////////////////////////////////////////////////
FormatFrameBasedDescriptor::FormatFrameBasedDescriptor(
	Device& _device, const void *data, int length)
	: FormatDescriptor(_device, data, length) {
}

uint8_t	FormatFrameBasedDescriptor::bBitsPerPixel() const {
	return uint8At(21);
}

uint8_t	FormatFrameBasedDescriptor::bDefaultFrameIndex() const {
	return uint8At(22);
}

uint8_t	FormatFrameBasedDescriptor::bAspectRatioX() const {
	return uint8At(23);
}

uint8_t	FormatFrameBasedDescriptor::bAspectRatioY() const {
	return uint8At(24);
}

uint32_t	FormatFrameBasedDescriptor::bmInterlaceFlags() const {
	return bitmapAt(25, 1);
}

uint8_t	FormatFrameBasedDescriptor::bCopyProtect() const {
	return uint8At(26);
}

std::string	FormatFrameBasedDescriptor::guidFormat() const {
	return std::string(5 + (char *)data);
}

static std::string	indent("        ");
static std::string	formatindent = indent + std::string("    FO  ");

std::string	FormatFrameBasedDescriptor::toString() const {
	std::ostringstream	out;
	out << indent << "Format FrameBased Descriptor:" << std::endl;
	out << formatindent << "bNumFrameDescriptors: ";
	out << (int)bNumFrameDescriptors() << std::endl;
	out << formatindent << "guidFormat:           ";
	out << guidFormat() << std::endl;
	out << formatindent << "bBitsPerPixel:        ";
	out << (int)bBitsPerPixel() << std::endl;
	out << formatindent << "bDefaultFrameIndex:   ";
	out << (int)bDefaultFrameIndex() << std::endl;
	out << formatindent << "bAspectRatioX:        ";
	out << (int)bAspectRatioX() << std::endl;
	out << formatindent << "bAspectRatioY:        ";
	out << (int)bAspectRatioY() << std::endl;
	out << formatindent << "bmInterlaceFlags:     ";
	out << std::hex << (int)bmInterlaceFlags() << std::endl;
	out << formatindent << "bCopyProtect:         ";
	out << (int)bCopyProtect() << std::endl;
	out << framesToString();
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// FrameFrameBasedDescriptor
//////////////////////////////////////////////////////////////////////
FrameFrameBasedDescriptor::FrameFrameBasedDescriptor(
	Device& _device, const void *data, int length)
	: FrameDescriptor(_device, data, length) {
}

uint32_t	FrameFrameBasedDescriptor::dwDefaultFrameInterval() const {
	return uint32At(17);
}

uint8_t	FrameFrameBasedDescriptor::bFrameIntervalType() const {
	return uint8At(21);
}

uint32_t	FrameFrameBasedDescriptor::dwBytesPerLine() const {
	return uint32At(22);
}

static std::string	frameindent = indent + std::string("    FRM ");

std::string	FrameFrameBasedDescriptor::toString() const {
	std::ostringstream	out;
	out << indent << "    " << "Frame FrameBased Descriptor:" << std::endl;
	out << this->FrameDescriptor::toString();
	out << frameindent << "  dwBytesPerLine:            ";
	out << dwBytesPerLine() << std::endl;
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
