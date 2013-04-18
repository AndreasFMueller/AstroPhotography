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
// FormatDescriptor
//////////////////////////////////////////////////////////////////////
FormatDescriptor::FormatDescriptor(Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
}

uint8_t	FormatDescriptor::bFormatIndex() const {
	return uint8At(3);
}

uint8_t	FormatDescriptor::bNumFrameDescriptors() const {
	return uint8At(4);
}

void	FormatDescriptor::setBNumFrameDescriptors(uint8_t b) {
	((uint8_t *)data)[4] = b;
}

static std::string	indent("            ");
static std::string	formatindent = indent + std::string("FO  ");
static std::string	frameindent = indent + std::string("    FRM ");

std::string	FormatDescriptor::toString() const {
	std::ostringstream	out;
	out << formatindent << "bFormatIndex:         ";
	out << (int)this->bFormatIndex() << std::endl;
	out << formatindent << "bNumFrameDescriptors: ";
	out << (int)this->bNumFrameDescriptors() << std::endl;
	out << formatindent << "bDefaultFrameIndex:   ";
	out << (int)this->bDefaultFrameIndex() << std::endl;
	out << formatindent << "bAspectRatioX:        ";
	out << (int)this->bAspectRatioX() << std::endl;
	out << formatindent << "bAspectRatioY:        ";
	out << (int)this->bAspectRatioY() << std::endl;
	out << formatindent << "bmInterlaceFlags:     ";
	out << std::hex << (int)this->bmInterlaceFlags() << std::endl;
	out << formatindent << "bCopyProtect:         ";
	out << (int)this->bCopyProtect() << std::endl;
	out << framesToString();
	return out.str();
}

std::string	FormatDescriptor::framesToString() const {
	std::ostringstream	out;
	out << formatindent << "Frames (" << numFrames() << "):" << std::endl;
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = frames.begin(); i != frames.end(); i++) {
		out << *i;
	}
	return out.str();
}

size_t	FormatDescriptor::numFrames() const {
	return frames.size();
}

const USBDescriptorPtr&	FormatDescriptor::operator[](size_t frameindex) const {
	if (frameindex >= numFrames()) {
		throw std::length_error("frameindex outside frame range");
	}
	return frames[frameindex];
}

int	FormatDescriptor::wTotalLength() const {
	std::vector<USBDescriptorPtr>::const_iterator	i;
	int	result = bLength();
	for (i = frames.begin(); i != frames.end(); i++) {
		result += (*i)->bLength();
	}
	return result;
}

bool	isFormatDescriptor(USBDescriptorPtr dp) {
	FormatDescriptor	*fdp = dynamic_cast<FormatDescriptor *>(&*dp);
	return (fdp != NULL) ? true : false;
}

//////////////////////////////////////////////////////////////////////
// FrameDescriptor
//////////////////////////////////////////////////////////////////////
FrameDescriptor::FrameDescriptor(Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
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

uint32_t	FrameDescriptor::minFrameInterval() const {
	if (0 == bFrameIntervalType()) {
		return dwMinFrameInterval();
	} else {
		return dwFrameInterval(0);
	}
}

std::string	FrameDescriptor::toString() const {
	std::ostringstream	out;
	out << frameindent << "bFrameIndex:               ";
	out << (int)this->bFrameIndex() << std::endl;
	out << frameindent << "bmCapabilities:            ";
	out << std::hex << this->bmCapabilities() << std::endl;
	out << frameindent << "wWidth:                    ";
	out << std::dec << this->wWidth() << std::endl;
	out << frameindent << "wHeight:                   ";
	out << std::dec << this->wHeight() << std::endl;
	out << frameindent << "dwMinBitRate:              ";
	out << this->dwMinBitRate() << std::endl;
	out << frameindent << "dwMaxBitRate:              ";
	out << this->dwMaxBitRate() << std::endl;
	out << frameindent << "dwDefaultFrameInterval:    ";
	out << this->dwDefaultFrameInterval() << std::endl;
	out << frameindent << "bFrameIntervalType:        ";
	out << ((this->bFrameIntervalType()) ? "discrete" : "continuous")
		<< std::endl;
	if (0 == bFrameIntervalType()) {
		out << frameindent << "    dwMinFrameInterval:    ";
		out << this->dwMinFrameInterval() << std::endl;
		out << frameindent << "    dwMaxFrameInterval:    ";
		out << this->dwMaxFrameInterval() << std::endl;
		out << frameindent << "    dwFrameIntervalStep:   ";
		out << this->dwFrameIntervalStep() << std::endl;
	} else {
		for (int index = 0; index < this->bFrameIntervalType(); index++) {
			out << frameindent << "    dwFrameInterval(" << index << ") = ";	
			out << this->dwFrameInterval(index) << std::endl;
		}
	}
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
