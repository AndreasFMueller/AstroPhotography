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

void	UVCCamera::selectFormatAndFrame(uint8_t interface,
		uint8_t format, uint8_t frame) throw(USBError) {
	// We want to negotiate use of the a given format and frame.
	// To do this, we send a SET probe
	vs_control_request_t	control_request;
	memset(&control_request, 0, sizeof(control_request));
	control_request.bFormatIndex = format;
	control_request.bFrameIndex = frame;
	Request<vs_control_request_t>	rset(REQUEST_CLASS_INTERFACE_SET,
		SET_CUR, VS_PROBE_CONTROL << 8, interface, &control_request);
	int	rc;
	if ((rc = devicehandle->controlRequest(&rset)) < 0) {
		throw USBError(libusb_error_name(rc));
	}

	// now probe the same thing, this should return a recommended
	// setting 
	Request<vs_control_request_t>	rget(REQUEST_CLASS_INTERFACE_GET,
		GET_CUR, VS_PROBE_CONTROL << 8, interface);
	if ((rc = devicehandle->controlRequest(&rget)) < 0) {
		throw USBError(libusb_error_name(rc));
	} else {
		if (rget.data()->bFormatIndex != format) {
			throw USBError("cannot negotiate format index");
		}
		if (rget.data()->bFrameIndex != frame) {
			throw USBError("cannot negotiate frame index");
		}
	}

	// if we get to this point, then negotiating the format and frame
	// was successful, and we can commit the negotiated paramters
	Request<vs_control_request_t>	rcommit(REQUEST_CLASS_INTERFACE_SET,
		SET_CUR, VS_COMMIT_CONTROL << 8, interface, rget.data());
	if ((rc = devicehandle->controlRequest(&rcommit)) < 0) {
		throw USBError(libusb_error_name(rc));
	}

#if 1
	std::cout << "Format:              ";
	std::cout << (int)rcommit.data()->bFormatIndex << std::endl;
	std::cout << "Frame:               ";
	std::cout << (int)rcommit.data()->bFrameIndex << std::endl;
	std::cout << "dwFrameInterval:     ";
	std::cout << (int)rcommit.data()->dwFrameInterval << std::endl;
	std::cout << "dwMaxVideoFrameSize: ";
	std::cout << (int)rcommit.data()->dwMaxVideoFrameSize << std::endl;
#endif
}

std::pair<uint8_t, uint8_t>	UVCCamera::getFormatAndFrame(uint8_t interface)
	throw(USBError) {
	Request<vs_control_request_t>	r(REQUEST_CLASS_INTERFACE_GET,
		GET_CUR, VS_PROBE_CONTROL << 8, interface);
	int	rc;
	if ((rc = devicehandle->controlRequest(&r)) < 0) {
		throw USBError(libusb_error_name(rc));
	}
	return std::make_pair(r.data()->bFormatIndex, r.data()->bFrameIndex);
}

int	UVCCamera::preferredAltSetting(uint8_t interface) {
	// XXX missing implementation
	return 5;
}

Frame	UVCCamera::getFrame(uint8_t ifno) {
	Frame	frame(NULL, 0);
	// get the interface 
	ConfigDescriptor	*config = device.config(0);
	const Interface& interface = config->interface(ifno);
	std::cout << interface;
	int	altsetting = 5;
	for (int alt = 1; alt < interface.numAltsettings(); alt++) {
		const InterfaceDescriptor& interfacedescriptor
			= interface[alt];
		std::cout << "alt setting " << (int)alt
		std::cout << ", wMaxPacketSize = ";
		std::cout << interfacedescriptor.endpoint()[0].wMaxPacketSize()
		std::cout << std::endl;
	}
	delete config;

	// now switch to the alternate setting for that interface (this
	// succeeds if the bandwidth can be negotiated
	int	rc = libusb_set_interface_alt_setting(devicehandle->dev_handle,
			ifno, altsetting);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	std::cout << "bandwidth negotiated" << std::endl;

	// now do the transfer with the interface

	// revert to alt setting 0, i.e. no data
	rc = libusb_set_interface_alt_setting(devicehandle->dev_handle,
			ifno, 0);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	std::cout << "bandwidth reset to 0" << std::endl;

	// XXX not implemented yet
	return	frame;
}

std::vector<Frame>	UVCCamera::getFrames(uint8_t interface, int nframes) {
	std::vector<Frame>	result;
	// XXX not implemented yet
	return result;
}

#if 0
typedef struct scanning_mode_control_s {
	typedef enum { CS = CT_SCANNINGMODE_CONTROL; } CS_type;
	uint8_t	bScanningMode;
} scanning_mode_control_t;
#endif

} // namespace uvc
} // namespace usb
} // namespace astro
