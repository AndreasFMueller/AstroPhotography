/*
 * UVCVideoControl.cpp
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
// InterfaceHeaderDescriptor
//////////////////////////////////////////////////////////////////////
InterfaceHeaderDescriptor::InterfaceHeaderDescriptor(Device& _device,
	const void *data, int length) : UVCDescriptor(_device, data, length) {
}

InterfaceHeaderDescriptor::~InterfaceHeaderDescriptor() {
}

uint16_t	InterfaceHeaderDescriptor::bcdUVC() const {
	return uint8At(3);
}

uint16_t	InterfaceHeaderDescriptor::wTotalLength() const {
	return uint16At(5);
}

uint32_t	InterfaceHeaderDescriptor::dwClockFrequency() const {
	return uint32At(7);
}

uint8_t	InterfaceHeaderDescriptor::bInCollection() const {
	return uint8At(11);
}

uint8_t	InterfaceHeaderDescriptor::baInterface(int index) const
	throw(std::range_error) {
	if ((index < 0) || (index >= bInCollection())) {
		throw std::range_error("baInterface(index) out of range");
	}
	return uint8At(12 + index);
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
	out << "  units:" << std::endl;
	for (int i = 0; i < numUnits(); i++) {
		out << operator[](i)->toString();
	}
	return out.str();
}

void	InterfaceHeaderDescriptor::getIds() {
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = units.begin(); i != units.end(); i++) {
		USBDescriptorPtr	dp = *i;
		if (isCameraTerminalDescriptor(dp)) {
			camera_terminal_id
				= cameraTerminalDescriptor(dp)->bTerminalID();
			camera_controls
				= cameraTerminalDescriptor(dp)->bmControls();
		}
                if (isProcessingUnitDescriptor(dp)) {
                        processing_unit_id
				= processingUnitDescriptor(dp)->bUnitID();
                        processing_unit_controls
				= processingUnitDescriptor(dp)->bmControls();
                }
        }
}

uint8_t	InterfaceHeaderDescriptor::cameraTerminalID() const {
	return camera_terminal_id;
}

uint32_t	InterfaceHeaderDescriptor::cameraControls() const {
	return camera_controls;
}

uint8_t	InterfaceHeaderDescriptor::processingUnitID() const {
	return processing_unit_id;
}

uint32_t	InterfaceHeaderDescriptor::processingUnitControls() const {
	return processing_unit_controls;
}

bool	isInterfaceHeaderDescriptor(const USBDescriptorPtr& ptr) {
	return (NULL == dynamic_cast<InterfaceHeaderDescriptor *>(&*ptr))
		? false : true;
}

InterfaceHeaderDescriptor *interfaceHeaderDescriptor(USBDescriptorPtr& ptr) {
	return dynamic_cast<InterfaceHeaderDescriptor *>(&*ptr);
}

const InterfaceHeaderDescriptor *interfaceHeaderDescriptor(
	const USBDescriptorPtr& ptr) {
	return dynamic_cast<const InterfaceHeaderDescriptor *>(&*ptr);
}

int	InterfaceHeaderDescriptor::numUnits() const {
	return units.size();
}

const USBDescriptorPtr&	InterfaceHeaderDescriptor::operator[](size_t index) const {
	return units[index];
}

//////////////////////////////////////////////////////////////////////
// TerminalDescriptor
//////////////////////////////////////////////////////////////////////

TerminalDescriptor::TerminalDescriptor(Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
}

uint8_t	TerminalDescriptor::bTerminalID() const {
	return uint8At(3);
}

uint16_t	TerminalDescriptor::wTerminalType() const {
	return uint16At(4);
}

uint8_t	TerminalDescriptor::bAssocTerminal() const {
	return uint8At(6);
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
InputTerminalDescriptor::InputTerminalDescriptor(Device& _device,
	const void *data, int length)
	: TerminalDescriptor(_device, data, length) {
	terminal = device.getStringDescriptor(((uint8_t *)data)[7]);
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

OutputTerminalDescriptor::OutputTerminalDescriptor(Device& _device,
	const void *data, int length)
	: TerminalDescriptor(_device, data, length) {
	terminal = device.getStringDescriptor(((uint8_t *)data)[8]);
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
	return uint8At(7);
}

//////////////////////////////////////////////////////////////////////
// CameraTerminalDescriptor
//////////////////////////////////////////////////////////////////////

CameraTerminalDescriptor::CameraTerminalDescriptor(Device& _device,
	const void *data, int length)
	: TerminalDescriptor(_device, data, length) {
	terminal = device.getStringDescriptor(((uint8_t *)data)[8]);
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
	out << "Camera Terminal Descriptor:" << std::endl;
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

bool	isCameraTerminalDescriptor(const USBDescriptorPtr& ptr) {
	return (NULL == dynamic_cast<CameraTerminalDescriptor *>(&*ptr))
		? false : true;
}

CameraTerminalDescriptor *cameraTerminalDescriptor(USBDescriptorPtr& ptr) {
	return dynamic_cast<CameraTerminalDescriptor *>(&*ptr);
}

//////////////////////////////////////////////////////////////////////
// SelectorUnitDescriptor
//////////////////////////////////////////////////////////////////////

SelectorUnitDescriptor::SelectorUnitDescriptor(Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
	int	p = bNrInPins();
	selector = device.getStringDescriptor(((uint8_t *)data)[5 + p]);
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
	return uint8At(5 + index);
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

ProcessingUnitDescriptor::ProcessingUnitDescriptor(Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
	int	n = bControlSize();
	processing = device.getStringDescriptor(((uint8_t *)data)[8 + n]);
}

uint8_t	ProcessingUnitDescriptor::bUnitID() const {
	return uint8At(3);
}

uint8_t	ProcessingUnitDescriptor::bSourceID() const {
	return uint8At(4);
}

uint16_t	ProcessingUnitDescriptor::wMaxMultiplier() const {
	return uint16At(5);
}

uint8_t	ProcessingUnitDescriptor::bControlSize() const {
	return uint8At(7);
}

uint32_t	ProcessingUnitDescriptor::bmControls() const {
	return bitmapAt(8, bControlSize());
}

uint32_t	ProcessingUnitDescriptor::bmVideoStandards() const {
	int	n = bControlSize();
	return bitmapAt(8 + n, 1);
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
	out << (int)bControlSize() << std::endl;
	out << "  bmControls:     ";
	uint32_t	bmcontrols = bmControls();
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
		out << " backlight_compensation";
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
	out << " (" << std::hex << (int)videostandards << ")";
	out << std::endl;
	return out.str();
}

bool	isProcessingUnitDescriptor(const USBDescriptorPtr& ptr) {
	return (NULL == dynamic_cast<ProcessingUnitDescriptor *>(&*ptr))
		? false : true;
}

ProcessingUnitDescriptor *processingUnitDescriptor(USBDescriptorPtr& ptr) {
	return dynamic_cast<ProcessingUnitDescriptor *>(&*ptr);
}

//////////////////////////////////////////////////////////////////////
// ExtensionUnitDescriptor
//////////////////////////////////////////////////////////////////////

ExtensionUnitDescriptor::ExtensionUnitDescriptor(Device& _device,
	const void *data, int length)
	: UVCDescriptor(_device, data, length) {
	int	p = bNrInPins();
	int	n = bControlSize();

	try {
		extension = device.getStringDescriptor(uint8At(23 + p + n));
	} catch (std::exception& x) {
		std::cerr << "extension naem not found: " << x.what()
			<< std::endl;
	}
	guid = std::string(&((char *)data)[4], 16);
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

} // namespace uvc
} // namespace usb
} // namespace astro
