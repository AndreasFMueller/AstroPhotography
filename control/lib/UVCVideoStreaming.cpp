/*
 * UVCVideoStreaming.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschuel Rappersil
 */
#include <AstroUVC.h>
#include <sstream>
#include <debug.h>

using namespace astro::usb;

namespace astro {
namespace usb {
namespace uvc {

//////////////////////////////////////////////////////////////////////
// HeaderDescriptor
//////////////////////////////////////////////////////////////////////
HeaderDescriptor::HeaderDescriptor(Device& _device,
	const void *_data, int length)
	: UVCDescriptor(_device, _data, length) {
	if (device.getBroken() == BROKEN_THE_IMAGING_SOURCE) {
std::cout << "fixing the imaging source:" << std::endl;
		((uint8_t *)data)[3] = (uint8_t)3;
		((uint8_t *)data)[4] = (uint8_t)231;
		((uint8_t *)data)[5] = (uint8_t)0;
	}
}

static std::string	indent = std::string("        ");
static std::string	headerindent = indent + std::string("H    ");

std::string	HeaderDescriptor::toString() const {
	std::ostringstream	out;
	out << headerindent << "bNumFormats:      ";
	out << (int)bNumFormats() << std::endl;
	out << headerindent << "wTotalLength:     ";
	out << wTotalLength() << std::endl;
	out << headerindent << "bEndpointAddress: ";
	out << std::hex << (int)bEndpointAddress() << std::endl;
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = formats.begin(); i != formats.end(); i++) {
		out << *i;
	}
	return out.str();
}

uint8_t	HeaderDescriptor::bNumFormats() const {
	return uint8At(3);
}

uint8_t	HeaderDescriptor::bEndpointAddress() const {
	return uint8At(6);
}

uint16_t	HeaderDescriptor::wTotalLength() const {
	return uint16At(4);
}

const USBDescriptorPtr	HeaderDescriptor::operator[](size_t formatindex) const {
	if ((formatindex < 0) || (formatindex >= bNumFormats())) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%d outside format range %d",
			formatindex, bNumFormats());
		throw std::length_error("outside format range");
	}
	return formats[formatindex];
}

//////////////////////////////////////////////////////////////////////
// InputHeaderDescriptor
//////////////////////////////////////////////////////////////////////
InputHeaderDescriptor::InputHeaderDescriptor(Device& _device,
	const void *data, int length)
	: HeaderDescriptor(_device, data, length) {
}

uint8_t	InputHeaderDescriptor::bmInfo() const {
	return uint8At(7);
}

uint8_t	InputHeaderDescriptor::bTerminalLink() const {
	return uint8At(8);
}

uint8_t	InputHeaderDescriptor::bStillCaptureMethod() const {
	return uint8At(9);
}

uint8_t	InputHeaderDescriptor::bTriggerSupport() const {
	return uint8At(10);
}

uint8_t	InputHeaderDescriptor::bTriggerUsage() const {
	return uint8At(11);
}

uint8_t	InputHeaderDescriptor::bControlSize() const {
	return uint8At(12);
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
	out << indent << "Input Header Descriptor:" << std::endl;
	out << headerindent << "bmInfo:              ";
	out << std::hex << (int)bmInfo() << std::endl;
	out << headerindent << "bTerminalLink:       ";
	out << std::dec << (int)bTerminalLink() << std::endl;
	out << headerindent << "bStillCaptureMethod: ";
	out << (int)bStillCaptureMethod() << std::endl;
	out << headerindent << "bTriggerSupport:     ";
	out << (int)bTriggerSupport() << std::endl;
	out << headerindent << "bTriggerUsage:       ";
	out << (int)bTriggerUsage() << std::endl;
	out << headerindent << "bControlSize:        ";
	out << (int)bControlSize() << std::endl;
	out << headerindent << "bmaControls:        ";
	for (int index = 0; index < bNumFormats(); index++) {
		out << " " << std::hex << bmaControls(index);
	}
	out << std::endl;
	out << this->HeaderDescriptor::toString();
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// OutputHeaderDescriptor
//////////////////////////////////////////////////////////////////////
OutputHeaderDescriptor::OutputHeaderDescriptor(Device& _device,
	const void *data, int length)
	: HeaderDescriptor(_device, data, length) {
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
	out << indent << "Output Header Descriptor:" << std::endl;
	out << headerindent << "bTerminalLink:       ";
	out << std::dec << (int)bTerminalLink() << std::endl;
	out << headerindent << "bControlSize:        ";
	out << (int)bControlSize() << std::endl;
	out << headerindent << "bmaControls:        ";
	for (int index = 0; index < bNumFormats(); index++) {
		out << " " << std::hex << bmaControls(index);
	}
	out << std::endl;
	out << this->HeaderDescriptor::toString();
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// VideoStreamingProbeControlRequest
//////////////////////////////////////////////////////////////////////
VideoStreamingProbeControlRequest::VideoStreamingProbeControlRequest(
	InterfacePtr interfaceptr, uint8_t bRequest,
	vs_control_request_t *_data)
	: Request<vs_control_request_t>(RequestBase::class_specific_type,
                interfaceptr, bRequest, VS_PROBE_CONTROL << 8, _data) {
	accept_short_response = true;
if (_data) {
std::cout << "bFormatIndex:  " << (int)(_data->bFormatIndex) << std::endl;
std::cout << "bFrameIndex:   " << (int)(_data->bFrameIndex) << std::endl;
}
std::cout << this->payloadHex();
}

VideoStreamingCommitControlRequest::VideoStreamingCommitControlRequest(
	InterfacePtr interfaceptr, uint8_t bRequest,
	vs_control_request_t *_data)
	: Request<vs_control_request_t>(RequestBase::class_specific_type,
                interfaceptr, bRequest, VS_COMMIT_CONTROL << 8, _data) {
	accept_short_response = true;
}

} // namespace uvc
} // namespace usb
} // namespace astro
