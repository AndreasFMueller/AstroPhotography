/*
 * UVCCamera.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <sstream>
#include <ostream>
#include <debug.h>

using namespace astro::usb::uvc;

namespace astro {
namespace usb {
namespace uvc {

/**
 * \brief Construct a camera from a USB Device
 */
UVCCamera::UVCCamera(Device& _device, bool force) throw(USBError)
	: device(_device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a UVC camera object");

	// make sure the camera is open, this most probably will not have
	// any effect
	device.open();

	// scan the active configuration for one that has an Interface
	// association descriptor
	ConfigurationPtr config = device.activeConfig();
	if (config->extra().size() == 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no extra descriptors");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Video Interface Association found");

	// get the control interface, and the list of interface descriptors
	// for the control interface, and claim it
	uint8_t	ci = controlInterfaceNumber();
	videocontrol = (*config)[ci];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Control interface number: %d", ci);
	videocontrol->detachKernelDriver();
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interfaces in association: %d",
		interfacecount);
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
	for (int vsif = controlInterfaceNumber() + 1;
		vsif < controlInterfaceNumber() + iad().bInterfaceCount();
		vsif++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"analyzing video streaming interface %d", vsif);
		InterfacePtr	interface = (*config)[vsif];
		// only alternate setting 0 contains the formats
		InterfaceDescriptorPtr	id = (*interface)[0];
		std::string	extra = id->extra();
		USBDescriptorPtr	vsd = vsf.descriptor(extra);
		videostreaming.push_back(vsd);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "UVCCamera constructed");
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

size_t	UVCCamera::streamingInterfaceNumber(size_t interfacenumber) const
	throw(std::range_error) {
	int	result = interfacenumber - controlInterfaceNumber() - 1;
	if ((result < 0) || (result >= videostreaming.size())) {
		throw std::range_error("outside video streaming interface range");
	}
	return result;
}

const USBDescriptorPtr&	UVCCamera::operator[](size_t interfacenumber) const 
	throw(std::range_error) {
	return videostreaming[streamingInterfaceNumber(interfacenumber)];
}

USBDescriptorPtr&	UVCCamera::operator[](size_t interfacenumber) 
	throw(std::range_error) {
	return videostreaming[streamingInterfaceNumber(interfacenumber)];
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

/**
 * \brief Get minimum frame interval for this interface/format/frame choice.
 *
 * When negotiating the bandwidth, we have to propose a frame interval.
 * We always use the minimum frame interval supported by the selected
 * frame descriptor. The camera can then still propose something larger,
 * which we would immediately accept.
 * \param interface	video streaming interface number, not the index in
 *			the videostreaming array.
 * \param format	format number, one larger than the format index
 * \param frame		frame number, equal to the bFrameIndex field of the
 *			frame descriptor
 */
uint32_t	UVCCamera::minFrameInterval(uint8_t interface, uint8_t format,
	uint8_t frame) throw(std::range_error,USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve minFrameInterval for "
		"interface = %d, format = %d, frame = %d",
		interface, format, frame);
	// we must indicate a frame interval during negotiation (for bandwith
	// computation purposes, so we need the descriptors
	USBDescriptorPtr	vsinterface = (*this)[interface];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got interface %d", interface);

	// this should be a header descriptor
	HeaderDescriptor	*header
		= dynamic_cast<HeaderDescriptor *>(&*vsinterface);
	if (NULL == header) {
		debug(LOG_ERR, DEBUG_LOG, 0, "VS header not found");
		throw USBError("not an VS header descriptor");
	}

	// now get the format descriptor
	USBDescriptorPtr	formatptr = (*header)[format - 1];
	FormatDescriptor	*formatdesc
		= dynamic_cast<FormatDescriptor *>(&*formatptr);
	if (NULL == formatdesc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "format descriptor %d not found",
			format);
		throw USBError("not a format descriptor");
	}

	// finally get the frame descriptor
	USBDescriptorPtr	frameptr = (*formatdesc)[frame - 1];
	FrameDescriptor	*framedesc
		= dynamic_cast<FrameDescriptor *>(&*frameptr);
	if (NULL == framedesc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "frame descriptor %d not found",
			frame);
		throw USBError("frame descriptor not found");
	}

	// we have found the minimum frame interval
	debug(LOG_DEBUG, DEBUG_LOG, 0, "minimum frame interval: %d",
		framedesc->minFrameInterval());
	return framedesc->minFrameInterval();
}

/**
 * \brief Select format and frame.
 *
 * This method negotiates format and frame with the device. This also
 * implies a frame interval setting, and the bandwith depends on this
 * setting as well. However, selecting the format and frame a priori 
 * does not yet fix the bandwidth, this is a consideration only for
 * isochronous transfers. Therefore this method does not do any bandwidth
 * negotiation, but leaves this to the getFrame method.
 *
 * \param interface	video streaming interface number, not the index in
 *			the videostreaming array.
 * \param format	format number, one larger than the format index
 * \param frame		frame number, equal to the bFrameIndex field of the
 *			frame descriptor
 */
void	UVCCamera::selectFormatAndFrame(uint8_t interface,
		uint8_t format, uint8_t frame) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"select interface %d, format %d, frame %d",
		interface, format, frame);
	// We want to negotiate use of the a given format and frame.
	// To do this, we send a SET probe
	vs_control_request_t	control_request;
	memset(&control_request, 0, sizeof(control_request));
	control_request.bFormatIndex = format;
	control_request.bFrameIndex = frame;
	control_request.dwFrameInterval
		= minFrameInterval(interface, format, frame);

	// do we have to claim the interface?
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d with %d alt settings",
		interfaceptr->interfaceNumber(),
		interfaceptr->numAltsettings());
#if 1
	interfaceptr->claim();
#endif
	VideoStreamingProbeControlRequest	rset(interfaceptr, SET_CUR,
							&control_request);
	device.controlRequest(&rset);

	// now probe the same thing, this should return a recommended
	// setting 
	VideoStreamingProbeControlRequest	rget(interfaceptr, GET_CUR);
	device.controlRequest(&rget);
	if (rget.data()->bFormatIndex != format) {
		throw USBError("cannot negotiate format index");
	}
	if (rget.data()->bFrameIndex != frame) {
		throw USBError("cannot negotiate frame index");
	}

	// if we get to this point, then negotiating the format and frame
	// was successful, and we can commit the negotiated paramters
	VideoStreamingCommitControlRequest	rcommit(interfaceptr, SET_CUR,
							rget.data());
	device.controlRequest(&rcommit);

	// just to be on the safe side, we should ask again what the
	// current settings are
	VideoStreamingCommitControlRequest	rcur(interfaceptr, GET_CUR);
	device.controlRequest(&rcur);
	if ((rcur.data()->bFormatIndex != format)
		|| (rcur.data()->bFrameIndex != frame)) {
		throw USBError("failed to set format an frame");
	}
#if 1
	std::cout << "Format:                   ";
	std::cout << (int)rcur.data()->bFormatIndex << std::endl;
	std::cout << "Frame:                    ";
	std::cout << (int)rcur.data()->bFrameIndex << std::endl;
	std::cout << "dwFrameInterval:          ";
	std::cout << (int)rcur.data()->dwFrameInterval << std::endl;
	std::cout << "dwMaxVideoFrameSize:      ";
	std::cout << (int)rcur.data()->dwMaxVideoFrameSize << std::endl;
	std::cout << "dwMaxPayloadTransferSize: ";
	std::cout << (int)rcur.data()->dwMaxPayloadTransferSize << std::endl;
#endif
}

/**
 * \brief Get current format and frame setting.
 *
 * \param interface	interface number of the streaming interface
 */
std::pair<uint8_t, uint8_t>	UVCCamera::getFormatAndFrame(uint8_t interface)
	throw(USBError) {
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	VideoStreamingProbeControlRequest	r(interfaceptr, GET_CUR);
	device.controlRequest(&r);
	return std::make_pair(r.data()->bFormatIndex, r.data()->bFrameIndex);
}

/**
 * \brief Compute which is the preferred alternate setting for this interface
 *
 * \param interface	interface number of the streaming interface
 */
int	UVCCamera::preferredAltSetting(uint8_t interface) {
	// get the currently negotiated settings
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	VideoStreamingProbeControlRequest	rget(interfaceptr, GET_CUR);
	device.controlRequest(&rget);

	// if the frame interval is 0, we have to ask the format for
	// the default frame interval
	uint32_t	dwFrameInterval = rget.data()->dwFrameInterval;
	if (0 == dwFrameInterval) {
		debug(LOG_WARNING, DEBUG_LOG, 0,
			"warning: no negotiated frame interval");
		dwFrameInterval = 333333;
	}

	// compute the data rate
	double	datarate = rget.data()->dwMaxVideoFrameSize
			* (10000000. / dwFrameInterval);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "required data rate: %.1fMBps",
		datarate / 1000000.);

	// for this software, bulk transfers are preferable, if the
	// device supports them, so we check whether alt setting 0 has a
	// bulk endpoint, and return 0 in that case
	InterfaceDescriptorPtr	ifdescptr = (*interfaceptr)[0];
	if (ifdescptr->numEndpoints() > 0) {
		EndpointDescriptorPtr	endpointptr = (*ifdescptr)[0];
		if (endpointptr->isBulk()) {
			return 0;
		}
	}

	// if there was no bulk endpoint, then we have to find an alternate
	// setting that provides enough bandwidth. So we go through all
	// the alternate settings and their endpoints and compute the 
	// bandwidth they provide. As soon as we find a suitable setting
	// we return that. Apparently cameras usually order alt settings
	// with increasing bandwidth, so this algorithm should be good
	// enough.
	for (int alt = 1; alt <= interfaceptr->numAltsettings(); alt++) {
		ifdescptr = (*interfaceptr)[alt];
		EndpointDescriptorPtr	endpointptr = (*ifdescptr)[0];
		size_t	maxbandwidth = endpointptr->maxBandwidth();
		if (maxbandwidth > datarate) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "first alt setting "
				"matching data rate %.1fMBps: %d (%.1fMBps)",
				datarate / 1000000., alt,
				maxbandwidth / 1000000.);
			return alt;
		}
	}

	// if we get to this point, then we did not find a suitable alternate
	// setting, then there is no way we can satisfy the bandwidth
	// requirements of this camera. so we have to throw an exception
	throw USBError("no alternate setting with enough bandwidth found");
}

/**
 * \brief Get a single frame.
 */
Frame	UVCCamera::getFrame(uint8_t ifno) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve a frame from if %d", ifno);
	Frame	frame(NULL, 0);

	// get the currently negotiated settings
	InterfacePtr	interfaceptr = (*device.activeConfig())[ifno];
	VideoStreamingProbeControlRequest	rget(interfaceptr, GET_CUR);
	device.controlRequest(&rget);

	// We have to claim the interface bevor we can actually use an
	// alternate setting
	interfaceptr->claim();

	// now switch to the alternate setting for that interface (this
	// succeeds if the bandwidth can be negotiated
	int	altsetting = preferredAltSetting(ifno);
	InterfaceDescriptorPtr	ifdescptr = (*interfaceptr)[altsetting];
	ifdescptr->altSetting();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bandwidth negotiation complete, "
		"alt setting: %d", altsetting);

	// get the Endpoint for this alternate setting
	EndpointDescriptorPtr	endpoint = (*ifdescptr)[0];

	// now do the transfer with this alt setting, for this we first have
	// to decide for how many microframes we want to transfer anything
	
	IsoTransfer	transfer(endpoint, 8000);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"create an IsoTransfer with 2000 packets");

	// submit this transfer to the device
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "submitting request");
		device.submit(&transfer);
	} catch (USBError& usberror) {
		debug(LOG_ERR, DEBUG_LOG, 0, "usb error: %s", usberror.what());
	}

	// revert to alt setting 0, i.e. no data
	ifdescptr = (*interfaceptr)[0];
	ifdescptr->altSetting();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bandwith reset to 0");

	// release the interface again
	try {
		interfaceptr->release();
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "release failed: %s", x.what());
	}

	// find the dimensions of the images
	int	width = 640;
	int	height = 480;

	// convert the retrieved data to an image
	std::list<IsoPacketPtr>::const_iterator	i;
	Frame	*currentframe = new Frame(640, 480);
	std::list<FramePtr>	frames;
	int	packetcounter = 0;
	int	processed = 0;
	int	framecounter = 0;
	bool	fid = false;
	for (i = transfer.packets.begin(); i != transfer.packets.end(); i++) {
		try {
			UVCIsoPacket	uvciso(**i);
			if (uvciso.fid() == fid) {
				if (NULL == currentframe) {
					currentframe = new Frame(width, height);
				}
				currentframe->append(uvciso.payload());
			} else {
				frames.push_back(FramePtr(currentframe));
				framecounter++;
				currentframe = NULL;
				fid = uvciso.fid();
			}
			processed++;
		} catch (std::exception& x) {
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "packet %d ignored: %s",
			//	packetcounter, x.what());
		}
		packetcounter++;
	}
	if (currentframe) {
		delete currentframe;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "processed packets: %d, frames: %d",
		processed, framecounter);

	// show how large the frames are:
	std::list<FramePtr>::const_iterator	j;
	framecounter = 0;
	for (j = frames.begin(); j != frames.end(); j++, framecounter++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "frame %d: %d bytes",
			framecounter, (*j)->size());
	}

	// XXX not implemented yet
	return	frame;
}

std::vector<Frame>	UVCCamera::getFrames(uint8_t interface, int nframes) {
	std::vector<Frame>	result;
	// XXX not implemented yet
	return result;
}

} // namespace uvc
} // namespace usb
} // namespace astro
