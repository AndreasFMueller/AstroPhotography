/*
 * UVCCamera.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <sstream>
#include <ostream>

using namespace astro::usb::uvc;

namespace astro {
namespace usb {
namespace uvc {

UVCCamera::UVCCamera(const Device& _device, bool force) : device(_device) {
	std::cout << "trying to build a UVCCamera object from a USB Device"
		<< std::endl;
	devicehandle = device.open();
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
	devicehandle->claimInterface(ci);
	int	interfacecount = iad().bInterfaceCount();
	int	counter = 1;
	while (counter < interfacecount) {
		devicehandle->claimInterface(ci + counter);
		counter++;
	}

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
	uint8_t	ci = controlInterface();
	devicehandle->releaseInterface(ci);
	int	interfacecount = iad().bInterfaceCount();
	int	counter = 1;
	while (counter < interfacecount) {
		devicehandle->releaseInterface(ci + counter);
		counter++;
	}
	delete devicehandle;
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

USBDescriptorPtr	UVCCamera::getFormat(uint8_t interface) {
	vs_control_request_t	r;

	r.header.bmRequestType = REQUEST_TYPE_GET;
	r.header.bRequest = GET_CUR;
	r.header.wValue = VS_PROBE_CONTROL << 8;
	r.header.wIndex = interface;
	r.header.wLength = 34;

	Request	request((usb_request_header_t *)&r); // creates a copy
	int	rc;
	if ((rc = devicehandle->controlRequest(request)) < 0) {
		throw USBError(libusb_error_name(rc));
	} else {
		request.copyTo((usb_request_header_t *)&r);
		int	formatindex = r.bFormatIndex;
		// find the descriptor for this format
		int	vsindex = interface - 1 - iad().bFirstInterface();
		USBDescriptorPtr	dptr = videostreaming[vsindex];
		HeaderDescriptor	*hd
			= dynamic_cast<HeaderDescriptor *>(&*dptr);
		if (NULL == hd) {
			throw std::runtime_error("format not found");
		}
		return (*hd)[formatindex - 1];
	}
}

void	UVCCamera::setFormat(int formatindex) {
	// XXX not implemented yet
}

USBDescriptorPtr	UVCCamera::getFrame(uint8_t interface) {
	// XXX not implemented yet
}

void	UVCCamera::setFrame(int frameindex) {
	// XXX not implemented yet
}

#if 0
Frame	UVCCamera::getFrame() {
	// XXX not implemented yet
}

std::vector<Frame>	getFrames(int nframes) {
	// XXX not implemented yet
}
#endif

} // namespace uvc
} // namespace usb
} // namespace astro
