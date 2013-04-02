/*
 * UVCCamera.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <sstream>

using namespace astro::usb::uvc;

namespace astro {
namespace usb {
namespace uvc {

UVCCamera::UVCCamera(const Device& _device, bool force) : device(_device) {
	std::cout << "trying to build a UVCCamera object from a USB Device" << std::endl;
	// scan the active configuration for one that has an Interface
	// association descriptor
	// XXX getting the active configuration leads to a segmentation fault
	//     so we do with configuration for the time being
	std::tr1::shared_ptr<ConfigDescriptor>	config
		= std::tr1::shared_ptr<ConfigDescriptor>(device.config(0));
	if (config->extra().size() == 0) {
		throw USBError("no InterfaceAssociationDescriptor");
	}

	// creating a descriptor factory
	UVCDescriptorFactory	f(device);
	std::vector<USBDescriptorPtr>	list
		= f.descriptors(config->extra());
	std::vector<USBDescriptorPtr>::const_iterator	i;
	bool	iadfound = false;

	// now scan all the descriptors for an interface association
	// descriptor
	for (i = list.begin(); i != list.end(); i++) {
		USBDescriptorPtr	dp = *i;
		InterfaceAssociationDescriptor	*iad
			= interfaceAssociationDescriptor(dp);
		if (NULL != iad) {
			if (force || (iad->isVideoInterfaceCollection())) {
				iadptr = dp;
				iadfound = true;
			}
		}
	}
	if (!iadfound) {
		throw USBError("no Video Interface Association found");
	}
	std::cerr << "Video Interface Association found" << std::endl;

	// get the control interface, and the list of interface descriptors
	// for the control interface
	uint8_t	ci = controlInterface();
	Interface	interface = config->interface(ci);
	InterfaceDescriptor	controlinterface = interface[0];

	// convert the additional descriptors into the control interface
	// descriptors
	VideoControlDescriptorFactory	vcf(device);
	std::string	extra = controlinterface.extra();
	videocontrol = vcf.descriptor(extra);
	if (!isInterfaceHeaderDescriptor(videocontrol)) {
		std::runtime_error("not a video control interface");
	}

	// now parse the video streaming interfaces
	VideoStreamingDescriptorFactory	vsf(device);
	for (int vsindex = 1; vsindex < iad().bInterfaceCount(); vsindex++) {
		Interface	interface = config->interface(vsindex);
		// only alternate setting 0 contains the formats
		InterfaceDescriptor	id = interface[0];
		std::string	extra = id.extra();
		USBDescriptorPtr	vsd = vsf.descriptor(extra);
		videostreaming.push_back(vsd);
	}
}

UVCCamera::~UVCCamera() {
}

const InterfaceAssociationDescriptor&	UVCCamera::iad() const {
	return dynamic_cast<const InterfaceAssociationDescriptor &>(*iadptr);
}

uint8_t	UVCCamera::controlInterface() const {
	return iad().bFirstInterface();
}

uint8_t	UVCCamera::controlCameraTerminalID() const {
	return interfaceHeaderDescriptor(videocontrol)->cameraTerminalID();
}

uint32_t	UVCCamera::controlCameraControls() const {
	return interfaceHeaderDescriptor(videocontrol)->cameraControls();
}

uint8_t	UVCCamera::controlProcessingUnitID() const {
	return interfaceHeaderDescriptor(videocontrol)->processingUnitID();
}

uint32_t	UVCCamera::controlProcessingUnitControls() const {
	return interfaceHeaderDescriptor(videocontrol)->processingUnitControls();
}

std::string	UVCCamera::toString() const {
	std::ostringstream	out;
	out << *device.config(0);
	out << iad();
	out << "Control interface:        ";
	out  << (int)controlInterface() << std::endl;
	out << "Camera Terminal ID:       ";
	out  << (int)controlCameraTerminalID() << std::endl;
	out << "Camera Controls:          ";
	out  << std::hex << controlCameraControls() << std::endl;
	out << "Processing Unit ID:       ";
	out  << std::dec << (int)controlProcessingUnitID() << std::endl;
	out << "Processing Unit Controls: ";
	out  << std::hex << controlProcessingUnitControls() << std::endl;
	out << videocontrol;
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = videostreaming.begin(); i != videostreaming.end(); i++) {
		out << *i;
	}
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const UVCCamera& camera) {
	return out << camera.toString();
}

USBDescriptorPtr	UVCCamera::getFormat() const {
	// XXX not implemented yet
}

void	UVCCamera::setFormat(int formatindex) {
	// XXX not implemented yet
}

USBDescriptorPtr	UVCCamera::getFrame() const {
	// XXX not implemented yet
}

void	UVCCamera::setFrame(int frameindex) {
	// XXX not implemented yet
}

Frame	UVCCamera::getFrame() {
	// XXX not implemented yet
}

std::vector<Frame>	getFrames(int nframes) {
	// XXX not implemented yet
}

} // namespace uvc
} // namespace usb
} // namespace astro
