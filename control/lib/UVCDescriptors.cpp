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
// UVCDescriptor
//////////////////////////////////////////////////////////////////////

UVCDescriptor::UVCDescriptor(const Device& device, const void *data, int length)
	: USBDescriptor(device, data, length) {
	bdescriptorsubtype = ((uint8_t *)data)[2];
}

UVCDescriptor::UVCDescriptor(const UVCDescriptor& other) : USBDescriptor(other) {
	bdescriptorsubtype = other.bdescriptorsubtype;
}

UVCDescriptor&	UVCDescriptor::operator=(const UVCDescriptor& other) {
	bdescriptorsubtype = other.bdescriptorsubtype;
	return *this;
}

uint8_t	UVCDescriptor::bDescriptorSubtype() const {
	return bdescriptorsubtype;
}

//////////////////////////////////////////////////////////////////////
// UVCDescriptorFactory
//////////////////////////////////////////////////////////////////////
UVCDescriptorFactory::UVCDescriptorFactory(const Device& _device)
	: DescriptorFactory(_device) {
}

USBDescriptorPtr	UVCDescriptorFactory::descriptor(const void *data, int length) throw(std::length_error, UnknownDescriptorError) {
	if (length < 2) {
		throw std::length_error("not engouth data for descriptor");
	}
	// check that there is enough data to process
	uint8_t	blength = ((uint8_t *)data)[0];
	if (blength > length) {
		throw std::length_error("not enough data for descriptor");
	}
	uint8_t	type = ((uint8_t *)data)[1];
	uint8_t	subtype = ((uint8_t *)data)[2];
	std::cerr << "type = " << (int)type << ", subtype = " << (int)subtype << std::endl;
	uint16_t	wterminaltype = *(uint16_t *)&(((uint8_t *)data)[4]);

	if (type == CS_INTERFACE) {
		switch (subtype) {
		case VC_HEADER:
			return USBDescriptorPtr(new InterfaceHeaderDescriptor(
				device, data, length));
			break;
		case VC_OUTPUT_TERMINAL:
			return USBDescriptorPtr(new OutputTerminalDescriptor(
				device, data, length));
			break;
		case VC_INPUT_TERMINAL:
			if (wterminaltype == ITT_CAMERA) {
				return USBDescriptorPtr(
					new CameraTerminalDescriptor(
						device, data, length));
			} else {
				return USBDescriptorPtr(
					new InputTerminalDescriptor(
						device, data, length));
			}
			break;
		case VC_SELECTOR_UNIT:
			return USBDescriptorPtr(new SelectorUnitDescriptor(
					device, data, length));
			break;
		case VC_PROCESSING_UNIT:
			return USBDescriptorPtr(new ProcessingUnitDescriptor(
					device, data, length));
			break;
		case VC_EXTENSION_UNIT:
			return USBDescriptorPtr(new ExtensionUnitDescriptor(
					device, data, length));
			break;
		}
	}

	if (type == CS_INTERFACE) {
		switch (subtype) {
		case VS_INPUT_HEADER:
			break;
		case VS_OUTPUT_HEADER:
			break;
		case VS_STILL_IMAGE_FRAME:
			break;
		case VS_COLORFORMAT:
			break;
		}
	}
	return DescriptorFactory::descriptor(data, length);
}

//////////////////////////////////////////////////////////////////////
// InterfaceHeaderDescriptor
//////////////////////////////////////////////////////////////////////
InterfaceHeaderDescriptor::InterfaceHeaderDescriptor(const Device& _device,
	const void *data, int length) : UVCDescriptor(_device, data, length) {
	uint8_t	*d = (uint8_t *)data;
	bcduvc = *(uint16_t *)&d[3];
	wtotallength = *(uint16_t *)&d[5];
	dwclockfrequency = *(uint32_t *)&d[7];
	bincollection = d[11];
	bainterface = new uint8_t[bincollection];
	memcpy(bainterface, &d[12], bincollection);
}

InterfaceHeaderDescriptor::InterfaceHeaderDescriptor(
	const InterfaceHeaderDescriptor& other)
	: UVCDescriptor(other), bcduvc(other.bcduvc),
		wtotallength(other.wtotallength),
		dwclockfrequency(other.dwclockfrequency),
		bincollection(other.bincollection) {
	bainterface = new uint8_t[bincollection];
	memcpy(bainterface, other.bainterface, bincollection);
}

InterfaceHeaderDescriptor::~InterfaceHeaderDescriptor() {
	delete[] bainterface;
}

InterfaceHeaderDescriptor&	InterfaceHeaderDescriptor::operator=(const InterfaceHeaderDescriptor& other) {
	bcduvc = other.bcduvc;
	wtotallength = other.wtotallength;
	dwclockfrequency = other.dwclockfrequency;
	bincollection = other.bincollection;
	bainterface = new uint8_t[bincollection];
	memcpy(bainterface, other.bainterface, bincollection);
}

uint16_t	InterfaceHeaderDescriptor::bcdUVC() const {
	return bcduvc;
}

uint16_t	InterfaceHeaderDescriptor::wTotalLength() const {
	return wtotallength;
}

uint32_t	InterfaceHeaderDescriptor::dwClockFrequency() const {
	return dwclockfrequency;
}

uint8_t	InterfaceHeaderDescriptor::bInCollection() const {
	return bincollection;
}

uint8_t	InterfaceHeaderDescriptor::baInterface(int index) const
	throw(std::range_error) {
	if ((index < 0) || (index >= bincollection)) {
		throw std::range_error("baInterface(index) out of range");
	}
	return bainterface[index];
}

std::string	InterfaceHeaderDescriptor::toString() const {
	std::ostringstream	out;
	out << "Interface Header:" << std::endl;
	out << "  bcdUVC:           ";
	out << std::hex << bcdUVC() << std::endl;
	out << "  wTotalLength:     ";
	out << std::dec << wTotalLength() << std::endl;
	out << "  dwClockFrequency: ";
	out << std::dec << dwClockFrequency() << std::endl;
	out << "  bInCollection:    ";
	out << std::dec << (int)bInCollection() << std::endl;
	out << "  baInterface:     ";
	for (int i = 0; i < bInCollection(); i++) {
		out << " " << std::hex << (int)baInterface(i);
	}
	out << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// TerminalDescriptor
//////////////////////////////////////////////////////////////////////

TerminalDescriptor::TerminalDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
}

TerminalDescriptor::TerminalDescriptor(
	const TerminalDescriptor &other) 
	: UVCDescriptor(other) {
}

TerminalDescriptor&	TerminalDescriptor::operator=(
	const TerminalDescriptor& other) {
	UVCDescriptor::operator=(other);
}

uint8_t	TerminalDescriptor::bTerminalID() const {
	return ((uint8_t *)data)[3];
}

uint16_t	TerminalDescriptor::wTerminalType() const {
	return *(uint16_t *)&(((uint8_t *)data)[4]);
}

uint8_t	TerminalDescriptor::bAssocTerminal() const {
	return ((uint8_t *)data)[6];
}

std::string	TerminalDescriptor::toString() const {
	std::ostringstream	out;
	out << "  bTerminalID:     ";
	out << (int)bTerminalID() << std::endl;
	out << "  wTerminalType:   ";
	out << std::hex << wTerminalType() << std::endl;
	out << "  bAssocTerminal:  ";
	out << std::dec << (int)bAssocTerminal() << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// InputTerminalDescriptor
//////////////////////////////////////////////////////////////////////
InputTerminalDescriptor::InputTerminalDescriptor(const Device& _device,
	const void *data, int length)
	: TerminalDescriptor(_device, data, length) {
	DeviceHandle	*handle = device.open();
	terminal = handle->getStringDescriptor(((uint8_t *)data)[7]);
	delete handle;
}

InputTerminalDescriptor::InputTerminalDescriptor(
	const InputTerminalDescriptor& other) : TerminalDescriptor(other) {
	terminal = other.terminal;
}

InputTerminalDescriptor&	InputTerminalDescriptor::operator=(
	const InputTerminalDescriptor& other) {
	TerminalDescriptor::operator=(other);
	terminal = other.terminal;
}

const std::string&	InputTerminalDescriptor::iTerminal() const {
	return terminal;
}

std::string	InputTerminalDescriptor::toString() const {
	std::ostringstream	out;
	out << "Input Terminal Descriptor:" << std::endl;
	out << this->TerminalDescriptor::toString();
	out << "  iTerminal:      " << terminal << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// OutputTerminalDescriptor
//////////////////////////////////////////////////////////////////////

OutputTerminalDescriptor::OutputTerminalDescriptor(const Device& _device,
	const void *data, int length)
	: TerminalDescriptor(_device, data, length) {
	DeviceHandle	*handle = device.open();
	terminal = handle->getStringDescriptor(((uint8_t *)data)[8]);
	delete handle;
}

OutputTerminalDescriptor::OutputTerminalDescriptor(
	const OutputTerminalDescriptor& other) : TerminalDescriptor(other) {
	terminal = other.terminal;
}

OutputTerminalDescriptor&	OutputTerminalDescriptor::operator=(
	const OutputTerminalDescriptor& other) {
	TerminalDescriptor::operator=(other);
	terminal = other.terminal;
}

const std::string&	OutputTerminalDescriptor::iTerminal() const {
	return terminal;
}

std::string	OutputTerminalDescriptor::toString() const {
	std::ostringstream	out;
	out << "Output Terminal Descriptor:" << std::endl;
	out << this->TerminalDescriptor::toString();
	out << "  bSourceID:      ";
	out << std::dec << (int)bSourceID() << std::endl;
	out << "  iTerminal:      " << terminal << std::endl;
	return out.str();
}

uint8_t	OutputTerminalDescriptor::bSourceID() const {
	return ((uint8_t *)data)[7];
}

//////////////////////////////////////////////////////////////////////
// CameraTerminalDescriptor
//////////////////////////////////////////////////////////////////////

CameraTerminalDescriptor::CameraTerminalDescriptor(const Device& _device,
	const void *data, int length)
	: TerminalDescriptor(_device, data, length) {
	DeviceHandle	*handle = device.open();
	terminal = handle->getStringDescriptor(((uint8_t *)data)[8]);
	delete handle;
}

CameraTerminalDescriptor::CameraTerminalDescriptor(
	const CameraTerminalDescriptor& other) : TerminalDescriptor(other) {
	terminal = other.terminal;
}

CameraTerminalDescriptor&	CameraTerminalDescriptor::operator=(
	const CameraTerminalDescriptor& other) {
	TerminalDescriptor::operator=(other);
	terminal = other.terminal;
}

const std::string&	CameraTerminalDescriptor::iTerminal() const {
	return terminal;
}

uint16_t	CameraTerminalDescriptor::wObjectiveFocalLengthMin() const {
	return *(uint16_t *)&(((uint8_t *)data)[8]);
}

uint16_t	CameraTerminalDescriptor::wObjectiveFocalLengthMax() const {
	return *(uint16_t *)&(((uint8_t *)data)[10]);
}

uint16_t	CameraTerminalDescriptor::wOcularFocalLength() const {
	return *(uint16_t *)&(((uint8_t *)data)[12]);
}

uint8_t	CameraTerminalDescriptor::bControlSize() const {
	return ((uint8_t *)data)[14];
}

uint32_t	CameraTerminalDescriptor::bmControls() const {
	uint32_t	result = *(uint16_t *)&(((uint8_t *)data)[15]);
	return result & 0x0007ffff;
}

std::string	CameraTerminalDescriptor::toString() const {
	std::ostringstream	out;
	out << "Camera Terminal Control:" << std::endl;
	out << this->TerminalDescriptor::toString();
	out << "  iTerminal:                ";
	out << iTerminal() << std::endl;
	out << "  wObjectiveFocalLengthMin: ";
	out << wObjectiveFocalLengthMin() << std::endl;
	out << "  wObjectiveFocalLengthMax: ";
	out << wObjectiveFocalLengthMax() << std::endl;
	out << "  wOcularFocalLength:       ";
	out << wOcularFocalLength() << std::endl;
	out << "  bControlSize:             ";
	out << (int)bControlSize() << std::endl;
	out << "  bmControls:              ";
	uint16_t	controls = bmControls();
	if (controls & (1 << 0)) {
		out << " scanning_mode";
	}
	if (controls & (1 << 1)) {
		out << " auto_exposure_mode";
	}
	if (controls & (1 << 2)) {
		out << " auto_exposure_priority";
	}
	if (controls & (1 << 3)) {
		out << " exposure_time_absolute";
	}
	if (controls & (1 << 4)) {
		out << " exposure_time_relative";
	}
	if (controls & (1 << 5)) {
		out << " focus_absolute";
	}
	if (controls & (1 << 6)) {
		out << " focus_relative";
	}
	if (controls & (1 << 7)) {
		out << " iris_absolute";
	}
	if (controls & (1 << 8)) {
		out << " iris_relative";
	}
	if (controls & (1 << 9)) {
		out << " zoom_absolute";
	}
	if (controls & (1 << 10)) {
		out << " zoom_relative";
	}
	if (controls & (1 << 11)) {
		out << " pantilt_absolute";
	}
	if (controls & (1 << 12)) {
		out << " pantilt_relative";
	}
	if (controls & (1 << 13)) {
		out << " roll_absolute";
	}
	if (controls & (1 << 14)) {
		out << " roll_relative";
	}
	if (controls & (1 << 15)) {
		out << " reserved";
	}
	if (controls & (1 << 16)) {
		out << " reserved";
	}
	if (controls & (1 << 17)) {
		out << " focus_dauto";
	}
	if (controls & (1 << 18)) {
		out << " privacy";
	}
	out << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// SelectorUnitDescriptor
//////////////////////////////////////////////////////////////////////

SelectorUnitDescriptor::SelectorUnitDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
	DeviceHandle	*handle = device.open();
	int	p = bNrInPins();
	selector = handle->getStringDescriptor(((uint8_t *)data)[5 + p]);
	delete handle;
}

SelectorUnitDescriptor::SelectorUnitDescriptor(const SelectorUnitDescriptor& other) 
	: UVCDescriptor(other) {
	selector = other.selector;
}

SelectorUnitDescriptor&	SelectorUnitDescriptor::operator=(const SelectorUnitDescriptor& other) {
	UVCDescriptor::operator=(other);
	selector = other.selector;
}

uint8_t	SelectorUnitDescriptor::bUnitID() const {
	return ((uint8_t *)data)[3];
}

uint8_t	SelectorUnitDescriptor::bNrInPins() const {
	return ((uint8_t *)data)[4];
}

uint8_t	SelectorUnitDescriptor::baSourceID(int index) const {
	if ((index < 0) || (index >= bNrInPins())) {
		throw std::range_error("out of selector unit pin range");
	}
	return ((uint8_t *)data)[5 + index];
}

std::string	SelectorUnitDescriptor::toString() const {
	std::ostringstream	out;
	out << "Selector Unit Descriptor:" << std::endl;
	out << "  bUnitID:      ";
	out << (int)bUnitID() << std::endl;
	out << "  bNrInPins:    ";
	out << (int)bNrInPins() << std::endl;
	out << "  baSourceID:  ";
	for (int i = 0; i < bNrInPins(); i++) {
		out << " " << (int)baSourceID(i);
	}
	out << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// ProcessingUnitDescriptor
//////////////////////////////////////////////////////////////////////

ProcessingUnitDescriptor::ProcessingUnitDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
	DeviceHandle	*handle = device.open();
	int	n = bControlSize();
	processing = handle->getStringDescriptor(((uint8_t *)data)[8 + n]);
	delete handle;
}

ProcessingUnitDescriptor::ProcessingUnitDescriptor(const ProcessingUnitDescriptor& other) : UVCDescriptor(other) {
	processing = other.processing;
}

ProcessingUnitDescriptor&	ProcessingUnitDescriptor::operator=(const ProcessingUnitDescriptor& other) {
	UVCDescriptor::operator=(other);
	processing = other.processing;
}

uint8_t	ProcessingUnitDescriptor::bUnitID() const {
	return ((uint8_t *)data)[3];
}

uint8_t	ProcessingUnitDescriptor::bSourceID() const {
	return ((uint8_t *)data)[4];
}

uint16_t	ProcessingUnitDescriptor::wMaxMultiplier() const {
	return *(uint16_t *)&(((uint8_t *)data)[5]);
}

uint8_t	ProcessingUnitDescriptor::bControlSize() const {
	return ((uint8_t *)data)[7];
}

uint32_t	ProcessingUnitDescriptor::bmControls() const {
	uint32_t	result = *(uint32_t *)&(((uint8_t *)data)[8]);
	return result & 0x0003ffff;
}

uint32_t	ProcessingUnitDescriptor::bmVideoStandards() const {
	int	n = bControlSize();
	uint8_t	result = ((uint8_t *)data)[9 + n];
	return 0x3f & result;
}

const std::string&	ProcessingUnitDescriptor::iProcessing() const {
	return processing;
}

std::string	ProcessingUnitDescriptor::toString() const {
	std::ostringstream	out;
	out << "Processing Unit Descriptor:" << std::endl;
	out << "  bUnitID:         ";
	out << (int)bUnitID() << std::endl;
	out << "  bSourceID:       ";
	out << (int)bSourceID() << std::endl;
	out << "  bControlSize:    ";
	out << "  bmControls:     ";
	uint32_t	bmcontrols;
	if (bmcontrols & (1 << 0)) {
		out << " brightness";
	}
	if (bmcontrols & (1 << 1)) {
		out << " contrast";
	}
	if (bmcontrols & (1 << 2)) {
		out << " hue";
	}
	if (bmcontrols & (1 << 3)) {
		out << " saturation";
	}
	if (bmcontrols & (1 << 4)) {
		out << " sharpness";
	}
	if (bmcontrols & (1 << 5)) {
		out << " gamma";
	}
	if (bmcontrols & (1 << 6)) {
		out << " white_balance_temperature";
	}
	if (bmcontrols & (1 << 7)) {
		out << " white_balance_component";
	}
	if (bmcontrols & (1 << 8)) {
		out << " bcklight_compensation";
	}
	if (bmcontrols & (1 << 9)) {
		out << " gain";
	}
	if (bmcontrols & (1 << 10)) {
		out << " power_line_frequency";
	}
	if (bmcontrols & (1 << 11)) {
		out << " hue_auto";
	}
	if (bmcontrols & (1 << 12)) {
		out << " white_balance_temperature_auto";
	}
	if (bmcontrols & (1 << 13)) {
		out << " white_balance_component_auto";
	}
	if (bmcontrols & (1 << 14)) {
		out << " digital_multiplier";
	}
	if (bmcontrols & (1 << 15)) {
		out << " digital_multiplier_limit";
	}
	if (bmcontrols & (1 << 16)) {
		out << " analog_video_standard";
	}
	if (bmcontrols & (1 << 17)) {
		out << " analog_video_lock_status";
	}
	out << std::endl;
	out << "  bmVideoStandards:";
	uint32_t	videostandards = bmVideoStandards();
	if (videostandards & (1 << 0)) {
		out << "none";
	}
	if (videostandards & (1 << 1)) {
		out << " NTSC-525/60";
	}
	if (videostandards & (1 << 2)) {
		out << " PAL-625/50";
	}
	if (videostandards & (1 << 3)) {
		out << " SECAM-625/50";
	}
	if (videostandards & (1 << 4)) {
		out << " NTSC-625/50";
	}
	if (videostandards & (1 << 5)) {
		out << " PAL-525/60";
	}
	out << std::endl;
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// ProcessingUnitDescriptor
//////////////////////////////////////////////////////////////////////

ExtensionUnitDescriptor::ExtensionUnitDescriptor(const Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
	DeviceHandle	*handle = device.open();
	int	p = bNrInPins();
	int	n = bControlSize();
	extension = handle->getStringDescriptor(((uint8_t *)data)[23 + p + n]);
	delete handle;
	guid = std::string(((char *)data)[4], 16);
}

ExtensionUnitDescriptor::ExtensionUnitDescriptor(const ExtensionUnitDescriptor& other) : UVCDescriptor(other) {
	extension = other.extension;
	guid = other.guid;
}

ExtensionUnitDescriptor&	ExtensionUnitDescriptor::operator=(const ExtensionUnitDescriptor& other) {
	UVCDescriptor::operator=(other);
	extension = other.extension;
	guid = other.guid;
}

uint8_t	ExtensionUnitDescriptor::bUnitID() const {
	return ((uint8_t *)data)[3];
}

uint8_t	ExtensionUnitDescriptor::bNumControls() const {
	return ((uint8_t *)data)[20];
}

uint8_t	ExtensionUnitDescriptor::bNrInPins() const {
	return ((uint8_t *)data)[21];
}

uint8_t	ExtensionUnitDescriptor::baSourceID(int index) const {
	int	p = bNrInPins();
	if ((index < 0) || (index > p)) {
		throw std::range_error("outside extension input pin range");
	}
	return ((uint8_t *)data)[22 + index];
}

uint8_t	ExtensionUnitDescriptor::bControlSize() const {
	int	p = bNrInPins();
	return ((uint8_t *)data)[22 + p];
}

uint32_t	ExtensionUnitDescriptor::bmControls() const {
	int	p = bNrInPins();
	int	n = bControlSize();
	uint32_t	mask = 0xff;
	for (int i = 2; i < n; i++) {
		mask |= mask << 8;
	}
	uint32_t	result = *(uint32_t *)&(((uint8_t *)data)[23 + p]);
	return result & mask;
}

std::string	ExtensionUnitDescriptor::toString() const {
	std::ostringstream	out;
	out << "Extension Unit Descriptor:" << std::endl;
	out << "  bUnitID:           ";
	out << (int)bUnitID() << std::endl;
	out << "  guidExtensionCode: ";
	out << guid << std::endl;
	out << "  bNumControls:      ";
	out << (int)bNumControls() << std::endl;
	out << "  bNrInPins:         ";
	out << (int)bNrInPins() << std::endl;
	out << "  baSourceID:       ";
	for (int i = 0; i < bNrInPins(); i++) {
		out << " " << (int)baSourceID(i);
	}
	out << std::endl;
	out << "  bControlSize:      ";
	out << (int)bControlSize() << std::endl;
	out << "  bmControls:        ";
	out << std::hex << bmControls() << std::endl;
	out << "  iExtension:        " << extension << std::endl;
	return out.str();
}

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
	return ((uint8_t *)data)[8];
}

uint8_t	OutputHeaderDescriptor::bControlSize() const {
	return ((uint8_t *)data)[8];
}

uint32_t	OutputHeaderDescriptor::bmaControls(int index) const {
	int	n = bControlSize();
	if ((index < 0) || (index >= bNumFormats())) {
		throw std::range_error("out of format range");
	}
	uint32_t	mask = 0xff;
	for (int i = 2; i < n; i++) {
		mask |= mask << 8;
	}
	uint32_t	result = *(uint32_t *)&(((uint8_t *)data)[9 + index * n]);
	return result & mask;
}

std::string	OutputHeaderDescriptor::toString() const {
	std::ostringstream	out;
	out << "Output Header Descriptor:" << std::endl;
	out << this->HeaderDescriptor::toString();
	out << "bTerminalLink:       ";
	out << std::dec << (int)bTerminalLink() << std::endl;
	out << "bControlSize:        ";
	out << (int)bControlSize() << std::endl;
	out << "bmaControls:        ";
	for (int index = 0; index < bNumFormats(); index++) {
		out << " " << std::hex << bmaControls(index);
	}
	out << std::endl;
	return out.str();
}

} // namespace uvc
} // namespace usb
} // namespace astro
