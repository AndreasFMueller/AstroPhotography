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

/**
 * \brief Construct a camera from a USB Device
 */
UVCCamera::UVCCamera(Device& _device, bool force) : device(_device) {
	std::cout << "trying to build a UVCCamera object from a USB Device"
		<< std::endl;
	// make sure the camera is open, this most probably will not have
	// any effect
	device.open();

	// scan the active configuration for one that has an Interface
	// association descriptor
	// XXX getting the active configuration leads to a segmentation fault
	//     so we do with configuration for the time being
	ConfigurationPtr config = device.config(0);
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
	// for the control interface, and claim it
	uint8_t	ci = controlInterfaceNumber();
	videocontrol = (*config)[ci];
	videocontrol->claim();	// XXX this is probably wrong. we should
				// only claim the interface if we really want
				// to use it

	// we also need to know all the video control descriptors appended
	// to this InterfaceDescriptor. The VideoControlDescriptorFactory
	// does that.
	InterfaceDescriptorPtr	controlinterface = (*videocontrol)[0];
	VideoControlDescriptorFactory	vcdf(device);
	videocontroldescriptors = vcdf.descriptors(controlinterface->extra());
	
	// now claim get the various interface descriptors, i.e. the
	// alternate settings for an interface
	int	interfacecount = iad().bInterfaceCount();
	int	counter = 1;
	while (counter < interfacecount) {
std::cout << "claiming interface " << (int)(ci + counter) << std::endl;
		device.claimInterface(ci + counter);
		counter++;
	}
std::cout << "done" << std::endl;

#if 0
	// convert the additional descriptors into the control interface
	// descriptors
	VideoControlDescriptorFactory	vcf(device);
	std::string	extra = controlinterface->extra();
	videocontrol = vcf.descriptor(extra);
	if (!isInterfaceHeaderDescriptor(videocontrol)) {
		std::runtime_error("not a video control interface");
	}

	// now parse the video streaming interfaces
	VideoStreamingDescriptorFactory	vsf(device);
	for (int vsindex = 1; vsindex < iad().bInterfaceCount(); vsindex++) {
		InterfacePtr	interface = config->interface(vsindex);
		// only alternate setting 0 contains the formats
		InterfaceDescriptor	id = interface[0];
		std::string	extra = id.extra();
		USBDescriptorPtr	vsd = vsf.descriptor(extra);
		videostreaming.push_back(vsd);
	}
#endif
}

UVCCamera::~UVCCamera() {
std::cout << "camera cleanup" << std::endl;
	uint8_t	ci = controlInterfaceNumber();
	device.releaseInterface(ci);
	int	interfacecount = iad().bInterfaceCount();
	int	counter = 1;
	while (counter < interfacecount) {
		device.releaseInterface(ci + counter);
		counter++;
	}
}

const InterfaceAssociationDescriptor&	UVCCamera::iad() const {
	return dynamic_cast<const InterfaceAssociationDescriptor &>(*iadptr);
}

uint8_t	UVCCamera::controlInterfaceNumber() const {
	return iad().bFirstInterface();
}

uint8_t	UVCCamera::controlCameraTerminalID() const {
	return interfaceHeaderDescriptor(videocontroldescriptors[0])->cameraTerminalID();
}

uint32_t	UVCCamera::controlCameraControls() const {
	return interfaceHeaderDescriptor(videocontroldescriptors[0])->cameraControls();
}

uint8_t	UVCCamera::controlProcessingUnitID() const {
	return interfaceHeaderDescriptor(videocontroldescriptors[0])->processingUnitID();
}

uint32_t	UVCCamera::controlProcessingUnitControls() const {
	return interfaceHeaderDescriptor(videocontroldescriptors[0])->processingUnitControls();
}

std::string	UVCCamera::toString() const {
	std::ostringstream	out;
	out << device.config(0);
	out << iad();
	out << "Control interface:        ";
	out  << (int)controlInterfaceNumber() << std::endl;
	out << "Camera Terminal ID:       ";
	out  << (int)controlCameraTerminalID() << std::endl;
	out << "Camera Controls:          ";
	out  << std::hex << controlCameraControls() << std::endl;
	out << "Processing Unit ID:       ";
	out  << std::dec << (int)controlProcessingUnitID() << std::endl;
	out << "Processing Unit Controls: ";
	out  << std::hex << controlProcessingUnitControls() << std::endl;
	out << videocontrol;
	std::vector<InterfacePtr>::const_iterator	i;
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
#if 0
	// We want to negotiate use of the a given format and frame.
	// To do this, we send a SET probe
	vs_control_request_t	control_request;
	memset(&control_request, 0, sizeof(control_request));
	control_request.bFormatIndex = format;
	control_request.bFrameIndex = frame;
	Request<vs_control_request_t>	rset(REQUEST_CLASS_INTERFACE_SET,
		SET_CUR, VS_PROBE_CONTROL << 8, interface, &control_request);
	int	rc;
	if ((rc = device.controlRequest(&rset)) < 0) {
		throw USBError(libusb_error_name(rc));
	}

	// now probe the same thing, this should return a recommended
	// setting 
	Request<vs_control_request_t>	rget(REQUEST_CLASS_INTERFACE_GET,
		GET_CUR, VS_PROBE_CONTROL << 8, interface);
	if ((rc = device.controlRequest(&rget)) < 0) {
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
	if ((rc = device.controlRequest(&rcommit)) < 0) {
		throw USBError(libusb_error_name(rc));
	}
#endif

#if 0
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
#if 0
	Request<vs_control_request_t>	r(REQUEST_CLASS_INTERFACE_GET,
		GET_CUR, VS_PROBE_CONTROL << 8, interface);
	int	rc;
	if ((rc = device.controlRequest(&r)) < 0) {
		throw USBError(libusb_error_name(rc));
	}
	return std::make_pair(r.data()->bFormatIndex, r.data()->bFrameIndex);
#endif
	return std::make_pair((uint8_t)0, (uint8_t)1);
}

int	UVCCamera::preferredAltSetting(uint8_t interface) {
	// get the currently negotiated settings
#if 0
	Request<vs_control_request_t>	rget(REQUEST_CLASS_INTERFACE_GET,
		GET_CUR, VS_PROBE_CONTROL << 8, interface);
	int	rc;
	if ((rc = device.controlRequest(&rget)) < 0) {
		throw USBError(libusb_error_name(rc));
	}

	// if the frame interval is 0, we have to ask the format for
	// the default frame interval
	int	dwFrameInterval = rget.data()->dwFrameInterval;
	if (0 == dwFrameInterval) {
		dwFrameInterval = 333333;
	}
	double	datapms = rget.data()->dwMaxVideoFrameSize
			* 10000000. / (333333 * 1000.);
	std::cout << "maxIsoPacketSize:    ";
	std::cout << device.maxIsoPacketSize(0x81) << std::endl;
	std::cout << "minimum packet size: ";
	std::cout << datapms << std::endl;
	std::cout << "speed:               ";
	std::cout << device.getDeviceSpeed() << std::endl;
#endif

	// XXX missing implementation
	return 5;
}

Frame	UVCCamera::getFrame(uint8_t ifno) {
	Frame	frame(NULL, 0);
#if 0
	// get the interface 
	ConfigurationPtr	config = device.config(0);
	InterfacePtr interface = (*config)[ifno];
	std::cout << *interface;
	int	altsetting = preferredAltSetting(ifno);
	for (int alt = 1; alt < interface->numAltsettings(); alt++) {
		InterfaceDescriptorPtr interfacedescriptor = (*interface)[alt];
		std::cout << "alt setting " << (int)alt;
		std::cout << ", wMaxPacketSize = ";
		std::cout << interfacedescriptor->endpoint()->wMaxPacketSize();
		std::cout << std::endl;
	}

	// now switch to the alternate setting for that interface (this
	// succeeds if the bandwidth can be negotiated
	int	rc = libusb_set_interface_alt_setting(device.dev_handle,
			ifno, altsetting);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	std::cout << "bandwidth negotiated" << std::endl;

	// now do the transfer with the interface
	preferredAltSetting(ifno);

	// revert to alt setting 0, i.e. no data
	rc = libusb_set_interface_alt_setting(device.dev_handle,
			ifno, 0);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
	std::cout << "bandwidth reset to 0" << std::endl;
#endif

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
