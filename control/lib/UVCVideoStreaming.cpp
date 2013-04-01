/*
 * UVCDescriptors.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschuel Rappersil
 */
#include <AstroUVC.h>
#include <sstream>

using namespace astro::usb;

namespace astro {
namespace usb {
namespace uvc {

//////////////////////////////////////////////////////////////////////
// HeaderDescriptor
//////////////////////////////////////////////////////////////////////
HeaderDescriptor::HeaderDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
}

HeaderDescriptor::HeaderDescriptor(const HeaderDescriptor& other) 
	: UVCDescriptor(other) {
}

HeaderDescriptor&       HeaderDescriptor::operator=(
	const HeaderDescriptor &other) {
	UVCDescriptor::operator=(other);
}

std::string	HeaderDescriptor::toString() const {
	std::ostringstream	out;
	out << "  bNumFormats:      " << (int)bNumFormats() << std::endl;
	out << "  wTotalLength:     " << wTotalLength() << std::endl;
	out << "  bEndpointAddress: " << (int)bEndpointAddress() << std::endl;
	return out.str();
}

uint8_t	HeaderDescriptor::bNumFormats() const {
	return ((uint8_t *)data)[3];
}

uint8_t	HeaderDescriptor::bEndpointAddress() const {
	return ((uint8_t *)data)[6];
}

uint16_t	HeaderDescriptor::wTotalLength() const {
	return *(uint16_t *)&(((uint8_t *)data)[4]);
}

//////////////////////////////////////////////////////////////////////
// InputHeaderDescriptor
//////////////////////////////////////////////////////////////////////
InputHeaderDescriptor::InputHeaderDescriptor(const Device& _device,
	const void *data, int length)
	: HeaderDescriptor(_device, data, length) {
}

InputHeaderDescriptor::InputHeaderDescriptor(const InputHeaderDescriptor& other) 
	: HeaderDescriptor(other) {
}

InputHeaderDescriptor&       InputHeaderDescriptor::operator=(
	const InputHeaderDescriptor &other) {
	HeaderDescriptor::operator=(other);
}

uint8_t	InputHeaderDescriptor::bmInfo() const {
	return ((uint8_t *)data)[7];
}

uint8_t	InputHeaderDescriptor::bTerminalLink() const {
	return ((uint8_t *)data)[8];
}

uint8_t	InputHeaderDescriptor::bStillCaptureMethod() const {
	return ((uint8_t *)data)[9];
}

uint8_t	InputHeaderDescriptor::bTriggerSupport() const {
	return ((uint8_t *)data)[10];
}

uint8_t	InputHeaderDescriptor::bTriggerUsage() const {
	return ((uint8_t *)data)[11];
}

uint8_t	InputHeaderDescriptor::bControlSize() const {
	return ((uint8_t *)data)[12];
}

uint32_t	InputHeaderDescriptor::bmaControls(int index) const {
	int	n = bControlSize();
	if ((index < 0) || (index >= bNumFormats())) {
		throw std::range_error("out of format range");
	}
	uint32_t	mask = 0xff;
	for (int i = 2; i < n; i++) {
		mask |= mask << 8;
	}
	uint32_t	result = *(uint32_t *)&(((uint8_t *)data)[13 + index * n]);
	return result & mask;
}

std::string	InputHeaderDescriptor::toString() const {
	std::ostringstream	out;
	out << "Input Header Descriptor:" << std::endl;
	out << this->HeaderDescriptor::toString();
	out << "bmInfo:              ";
	out << std::hex << (int)bmInfo() << std::endl;
	out << "bTerminalLink:       ";
	out << std::dec << (int)bTerminalLink() << std::endl;
	out << "bStillCaptureMethod: ";
	out << (int)bStillCaptureMethod() << std::endl;
	out << "bTriggerSupport:     ";
	out << (int)bTriggerSupport() << std::endl;
	out << "bTriggerUsage:       ";
	out << (int)bTriggerUsage() << std::endl;
	out << "bControlSize:        ";
	out << (int)bControlSize() << std::endl;
	out << "bmaControls:        ";
	for (int index = 0; index < bNumFormats(); index++) {
		out << " " << std::hex << bmaControls(index);
	}
	out << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// OutputHeaderDescriptor
//////////////////////////////////////////////////////////////////////
OutputHeaderDescriptor::OutputHeaderDescriptor(const Device& _device,
	const void *data, int length)
	: HeaderDescriptor(_device, data, length) {
}

OutputHeaderDescriptor::OutputHeaderDescriptor(const OutputHeaderDescriptor& other) 
	: HeaderDescriptor(other) {
}

OutputHeaderDescriptor&       OutputHeaderDescriptor::operator=(
	const OutputHeaderDescriptor &other) {
	HeaderDescriptor::operator=(other);
}

uint8_t	OutputHeaderDescriptor::bTerminalLink() const {
	return uint8At(7);
}

uint8_t	OutputHeaderDescriptor::bControlSize() const {
	return uint8At(8);
}

uint32_t	OutputHeaderDescriptor::bmaControls(int index) const {
	int	n = bControlSize();
	return bitmapAt(9 + index * n, n);
}

std::string	OutputHeaderDescriptor::toString() const {
	std::ostringstream	out;
	out << "Output Header Descriptor:" << std::endl;
	out << this->HeaderDescriptor::toString();
	out << "  bTerminalLink:       ";
	out << std::dec << (int)bTerminalLink() << std::endl;
	out << "  bControlSize:        ";
	out << (int)bControlSize() << std::endl;
	out << "  bmaControls:        ";
	for (int index = 0; index < bNumFormats(); index++) {
		out << " " << std::hex << bmaControls(index);
	}
	out << std::endl;
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
