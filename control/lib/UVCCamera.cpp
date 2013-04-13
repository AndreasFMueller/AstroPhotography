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
UVCCamera::UVCCamera(Device& _device, bool force) throw(USBError)
	: device(_device) {
	std::cout << "trying to build a UVCCamera object from a USB Device"
		<< std::endl;
	// make sure the camera is open, this most probably will not have
	// any effect
	device.open();

	// scan the active configuration for one that has an Interface
	// association descriptor
	ConfigurationPtr config = device.activeConfig();
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
#if 0
	videocontrol->claim();	// XXX this is probably wrong. we should
				// only claim the interface if we really want
				// to use it
#endif

	// we also need to know all the video control descriptors appended
	// to this InterfaceDescriptor. The VideoControlDescriptorFactory
	// does that.
	InterfaceDescriptorPtr	controlinterface = (*videocontrol)[0];
	VideoControlDescriptorFactory	vcdf(device);
	videocontroldescriptors = vcdf.descriptors(controlinterface->extra());
	//std::cout << videocontroldescriptors[0];
	
	// now claim get the various interface descriptors, i.e. the
	// alternate settings for an interface
	int	interfacecount = iad().bInterfaceCount();
	int	counter = 1;
	while (counter < interfacecount) {
#if 0
std::cout << "claiming interface " << (int)(ci + counter) << std::endl;
		device.claimInterface(ci + counter);
#endif
		counter++;
	}

	// now parse the video streaming interfaces
	VideoStreamingDescriptorFactory	vsf(device);
	for (int vsindex = controlInterfaceNumber() + 1;
		vsindex < iad().bInterfaceCount(); vsindex++) {
		InterfacePtr	interface = (*config)[vsindex];
		// only alternate setting 0 contains the formats
		InterfaceDescriptorPtr	id = (*interface)[0];
		std::string	extra = id->extra();
		USBDescriptorPtr	vsd = vsf.descriptor(extra);
		videostreaming.push_back(vsd);
	}
}

UVCCamera::~UVCCamera() {
std::cout << "camera cleanup" << std::endl;
	try {
#if 0
		uint8_t	ci = controlInterfaceNumber();
		device.releaseInterface(ci);
		int	interfacecount = iad().bInterfaceCount();
		int	counter = 1;
		while (counter < interfacecount) {
			device.releaseInterface(ci + counter);
			counter++;
		}
#endif
		device.close();
	} catch (std::exception& x) {
		std::cout << "error during cleanup: " << x.what() << std::endl;
		throw;
	}
std::cout << "camera cleanup complete" << std::endl;
}

const InterfaceAssociationDescriptor&	UVCCamera::iad() const {
	return dynamic_cast<const InterfaceAssociationDescriptor &>(*iadptr);
}

const USBDescriptorPtr&	UVCCamera::operator[](size_t interfacenumber) const {
	return videostreaming[interfacenumber - controlInterfaceNumber() - 1];
}

USBDescriptorPtr&	UVCCamera::operator[](size_t interfacenumber) {
	return videostreaming[interfacenumber - controlInterfaceNumber() - 1];
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
	out << *device.activeConfig();
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
	out << *videocontrol;
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = videostreaming.begin(); i != videostreaming.end(); i++) {
		out << **i;
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

	// do we have to claim the interface?
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
#if 0
	interfaceptr->claim();
#endif

	Request<vs_control_request_t>	rset(RequestBase::class_specific_type,
		interfaceptr, SET_CUR, VS_PROBE_CONTROL << 8, &control_request);
	device.controlRequest(&rset);

	// now probe the same thing, this should return a recommended
	// setting 
	Request<vs_control_request_t>	rget(RequestBase::class_specific_type,
		interfaceptr, GET_CUR, VS_PROBE_CONTROL << 8);
	device.controlRequest(&rget);
	if (rget.data()->bFormatIndex != format) {
		throw USBError("cannot negotiate format index");
	}
	if (rget.data()->bFrameIndex != frame) {
		throw USBError("cannot negotiate frame index");
	}

	// if we get to this point, then negotiating the format and frame
	// was successful, and we can commit the negotiated paramters
	Request<vs_control_request_t> rcommit(RequestBase::class_specific_type,
		interfaceptr, SET_CUR, VS_COMMIT_CONTROL << 8, rget.data());
	device.controlRequest(&rcommit);

	// just to be on the safe side, we should ask again what the
	// current settings are
	Request<vs_control_request_t>	rcur(RequestBase::class_specific_type,
		interfaceptr, GET_CUR, VS_COMMIT_CONTROL << 8);
	device.controlRequest(&rcur);
	if ((rcur.data()->bFormatIndex != format)
		|| (rcur.data()->bFrameIndex != frame)) {
		throw USBError("failed to set format an frame");
	}
#if 1
	std::cout << "Format:              ";
	std::cout << (int)rcur.data()->bFormatIndex << std::endl;
	std::cout << "Frame:               ";
	std::cout << (int)rcur.data()->bFrameIndex << std::endl;
	std::cout << "dwFrameInterval:     ";
	std::cout << (int)rcur.data()->dwFrameInterval << std::endl;
	std::cout << "dwMaxVideoFrameSize: ";
	std::cout << (int)rcur.data()->dwMaxVideoFrameSize << std::endl;
#endif
}

std::pair<uint8_t, uint8_t>	UVCCamera::getFormatAndFrame(uint8_t interface)
	throw(USBError) {
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	Request<vs_control_request_t>	r(RequestBase::class_specific_type,
		interfaceptr, GET_CUR, VS_PROBE_CONTROL << 8);
	device.controlRequest(&r);
	return std::make_pair(r.data()->bFormatIndex, r.data()->bFrameIndex);
}

int	UVCCamera::preferredAltSetting(uint8_t interface) {
	// get the currently negotiated settings
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	Request<vs_control_request_t>	rget(RequestBase::class_specific_type,
		interfaceptr, GET_CUR, VS_PROBE_CONTROL << 8);
	device.controlRequest(&rget);

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

	// XXX missing implementation
	return 5;
}

Frame	UVCCamera::getFrame(uint8_t ifno) {
	Frame	frame(NULL, 0);

	// get the currently negotiated settings
	InterfacePtr	interfaceptr = (*device.activeConfig())[ifno];
	Request<vs_control_request_t>	rget(RequestBase::class_specific_type,
		interfaceptr, GET_CUR, VS_PROBE_CONTROL << 8);
	device.controlRequest(&rget);

	// show the alt settings of the interface
	std::cout << *interfaceptr;
	for (int alt = 1; alt < interfaceptr->numAltsettings(); alt++) {
		InterfaceDescriptorPtr interfacedescriptor = (*interfaceptr)[alt];
		std::cout << "alt setting " << (int)alt;
		std::cout << ", wMaxPacketSize = ";
		std::cout << (*interfacedescriptor)[0]->wMaxPacketSize();
		std::cout << std::endl;
	}

	// now switch to the alternate setting for that interface (this
	// succeeds if the bandwidth can be negotiated
	int	altsetting = preferredAltSetting(ifno);
	InterfaceDescriptorPtr	ifdescptr = (*interfaceptr)[altsetting];
	ifdescptr->altSetting();
	std::cout << "bandwidth negotiated" << std::endl;

	// now do the transfer with this alt setting

	// revert to alt setting 0, i.e. no data
	ifdescptr = (*interfaceptr)[0];
	ifdescptr->altSetting();
	std::cout << "bandwidth reset to 0" << std::endl;

	// convert the retrieved data to an image
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
