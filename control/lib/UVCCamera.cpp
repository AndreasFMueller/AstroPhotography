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



bool	isUVCDevice(Device& device) {
	// handle the special case of TIS camera
	DeviceDescriptorPtr	devicedescriptor = device.descriptor();
	if (VENDOR_THE_IMAGING_SOURCE == devicedescriptor->idVendor()) {
		return true;
	}

	// for all other devices, look for a interface association descriptor
	// for a video interface
	
	return true;
}

/**
 * \brief Get current settings of a interface.
 *
 * This method determines some often used variables. Since we use this
 * same code from multiple places, we do it in this method.
 * \param interface	streaming interface number from which we want to
 *			get the default settings
 */
void	UVCCamera::getCur(uint8_t interface) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get current settings of interface %d",
		interface);

	// get the interface descriptor
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];

	// get the current streaming settings of that interface
	VideoStreamingCommitControlRequest	rcur(interfaceptr, GET_CUR);
	device.controlRequest(&rcur);

	// also set frame dimensions from the currently active frame descriptor
	FrameDescriptor	*framedescriptor
		= getPtr<FrameDescriptor>(getFrameDescriptor(interface,
			rcur.data()->bFormatIndex, rcur.data()->bFrameIndex));
	width = framedescriptor->wWidth();
	height = framedescriptor->wHeight();

	// get the frame interval
	frameinterval = rcur.data()->dwFrameInterval;
	maxvideoframesize = rcur.data()->dwMaxVideoFrameSize;
	maxpayloadtransfersize = rcur.data()->dwMaxPayloadTransferSize;

	if (debuglevel >= LOG_DEBUG) {
		std::cout << "Format:                   ";
		std::cout << (int)rcur.data()->bFormatIndex << std::endl;
		std::cout << "Frame:                    ";
		std::cout << (int)rcur.data()->bFrameIndex << std::endl;
		std::cout << "wWidth:                   ";
		std::cout << (int)framedescriptor->wWidth() << std::endl;
		std::cout << "wHeight:                  ";
		std::cout << (int)framedescriptor->wHeight() << std::endl;
		std::cout << "dwFrameInterval:          ";
		std::cout << (int)frameinterval << std::endl;
		std::cout << "dwMaxVideoFrameSize:      ";
		std::cout << (int)maxvideoframesize << std::endl;
		std::cout << "dwMaxPayloadTransferSize: ";
		std::cout << maxpayloadtransfersize << std::endl;
	}
}

/**
 * \brief Construct a camera from a USB Device
 *
 * The constructor undertakes an extensive analysis of the descriptors
 * to find the video control and video streaming interfaces of the video
 * function of the device. It also makes sure no kernel driver is attached
 * to the device. It does not, however, claim any of the interfaces, this
 * is done when the device is really used.
 * \param _device	an USB device to open as a UVC camera
 * \param force		force opening as camera even if the
 *			interface associaten descriptor does not
 *			declare itself as a video interface association
 *                      descriptor (handles the TIS camera)
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

	// get the list of interface association descriptors
	std::list<USBDescriptorPtr>	iadlist
		= device.interfaceAssociationDescriptors(true);
	if (0 == iadlist.size()) {
		throw USBError("no Video Interface Association found");
	}
	iadptr = *iadlist.begin();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Video Interface Association found");

	// get the control interface, and the list of interface descriptors
	// for the control interface, and claim it
	uint8_t	ci = controlInterfaceNumber();
	videocontrol = (*config)[ci];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Control interface number: %d", ci);
	videocontrol->detachKernelDriver();

	// we also need to know all the video control descriptors appended
	// to this InterfaceDescriptor. The VideoControlDescriptorFactory
	// does that.
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parse the video control descriptors");
	InterfaceDescriptorPtr	controlinterface = (*videocontrol)[0];
	VideoControlDescriptorFactory	vcdf(device);
	videocontroldescriptors = vcdf.descriptors(controlinterface->extra());
	std::cout << videocontroldescriptors[0];
	
	// now claim get the various interface descriptors, i.e. the
	// alternate settings for an interface
	int	interfacecount = iad().bInterfaceCount();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interfaces in association: %d",
		interfacecount);

	// now parse the video streaming interfaces
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parse streaming interface descriptors");
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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "extra descriptors: %d bytes",
			extra.size());
		USBDescriptorPtr	vsd = vsf.descriptor(extra);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "parse complete");
		videostreaming.push_back(vsd);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "UVCCamera constructed");
}

/**
 * \brief Destroy the camera.
 *
 * This essentially closes the video interface. Note that it does not
 * reinstall a kernel driver if one was attached.
 */
UVCCamera::~UVCCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera cleanup");
	try {
		device.close();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during cleanup: %s",
			x.what());
		throw x;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera cleanup complete");
}

/**
 * \brief Get the header descriptor for this streaming interface.
 *
 * This function verifies that the interface is really a streaming interface
 * and returns the interface descriptor for it.
 * \param interface interface number for the streaming interface
 */
USBDescriptorPtr	UVCCamera::getHeaderDescriptor(uint8_t interface) {
	USBDescriptorPtr	headerptr = (*this)[interface];
	if (!isPtr<HeaderDescriptor>(headerptr)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a header descriptor");
		throw std::runtime_error("not a header descriptor");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found a header descriptor");
	return headerptr;
}

/**
 * \brief Get the format descriptor with this format index
 *
 * \param interface      USB interface number of the streaming interfae
 * \param formatindex    format index (first format has index 1)
 */ 
USBDescriptorPtr	UVCCamera::getFormatDescriptor(uint8_t interface,
	uint8_t formatindex) {
	// logging
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get format for interface = %d, "
		"format = %d", interface, formatindex);

	// get the header descriptor
	USBDescriptorPtr	header = getHeaderDescriptor(interface);
	HeaderDescriptor	*headerptr = getPtr<HeaderDescriptor>(header);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "headerptr = %p", headerptr);

	// we don't need to do any range checking on the formatindex
	// because the operator[] of the header descriptor class will do it
	USBDescriptorPtr	formatptr = (*headerptr)[formatindex - 1];
	if (!isPtr<FormatDescriptor>(formatptr)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a format descriptor");
		throw std::runtime_error("not a format descriptor");
	}

	// if we get to this point, then we have found the format
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found format");
	return formatptr;
}

/**
 * \brief Get the frame descriptor of the currently selected frame
 *
 * \param interface      USB interface number of the streaming interfae
 * \param formatindex    format index (first format has index 0)
 * \param frameindex	 frame index (first frame has index 0)
 */
USBDescriptorPtr	UVCCamera::getFrameDescriptor(uint8_t interface,
	uint8_t formatindex, uint8_t frameindex) {
	// logging
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get frame descriptor interface = %d, "
		"format = %d, frame = %d", interface, formatindex, frameindex);

	// get a format pointer
	USBDescriptorPtr	format = getFormatDescriptor(interface,
		formatindex);
	FormatDescriptor	*formatptr = getPtr<FormatDescriptor>(format);

	// get the frame pointer from the format
	USBDescriptorPtr	frameptr = (*formatptr)[frameindex - 1];
	if (!isPtr<FrameDescriptor>(frameptr)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not a frame descriptor");
		throw std::runtime_error("not a frame descriptor");
	}

	// if we get here, we have found the frame
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found frame");
	return frameptr;
}

/**
 * \brief Get the interface association descriptor for the video function
 *
 */
const InterfaceAssociationDescriptor&	UVCCamera::iad() const {
	return *getPtr<InterfaceAssociationDescriptor>(iadptr);
}

/**
 * \brief Get the index of the streaming interface in the videostreaming
 *        vector.
 *
 * Internally we need the index into the video streaming. 
 * \param interfacenumber    USB interface number for this streaming interface
 */
size_t	UVCCamera::streamingInterfaceIndex(size_t interfacenumber) const
	throw(std::range_error) {
	uint8_t	cifn = controlInterfaceNumber();
	if (interfacenumber <= cifn) {
		throw std::range_error("interface number too small");
	}
	unsigned int	result = interfacenumber - cifn - 1;
	if (result >= videostreaming.size()) {
		throw std::range_error("outside VS interface range");
	}
	return result;
}

size_t	UVCCamera::numberVideoStreamingInterfaces() const {
	return iad().bInterfaceCount() - 1;
}

const USBDescriptorPtr&	UVCCamera::operator[](size_t interfacenumber) const 
	throw(std::range_error) {
	return videostreaming[streamingInterfaceIndex(interfacenumber)];
}

USBDescriptorPtr&	UVCCamera::operator[](size_t interfacenumber) 
	throw(std::range_error) {
	return videostreaming[streamingInterfaceIndex(interfacenumber)];
}

uint8_t	UVCCamera::controlInterfaceNumber() const {
	return iad().bFirstInterface();
}

InterfaceHeaderDescriptor	*UVCCamera::interfaceHeaderDescriptor() const {
	return getPtr<InterfaceHeaderDescriptor>(videocontroldescriptors[0]);
}

uint8_t	UVCCamera::controlCameraTerminalID() const {
	return interfaceHeaderDescriptor()->cameraTerminalID();
}

uint32_t	UVCCamera::controlCameraControls() const {
	return interfaceHeaderDescriptor()->cameraControls();
}

uint8_t	UVCCamera::controlProcessingUnitID() const {
	return interfaceHeaderDescriptor()->processingUnitID();
}

uint32_t	UVCCamera::controlProcessingUnitControls() const {
	return interfaceHeaderDescriptor()->processingUnitControls();
}

/**
 * \brief Set exposure time
 *
 * This sets the exposure priority as well
 */
void	UVCCamera::setExposureTime(double exposuretime) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting auto exposure priority");

	// we have to find out whether the auto_exposure_priority_control
	// is available on this camera
	astro::usb::uvc::auto_exposure_priority_control_t       aeprio;
	// make sure time has priority
	// bAutoExposurePriority == 1 means that frame rate may be
	// altered dynamically
	aeprio.bAutoExposurePriority = 1;
	if (controlSupported(aeprio)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"auto exposure priority control supported");
		setCurrent(aeprio);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting auto exposure mode");
	// set the auto exposure mode
	astro::usb::uvc::auto_exposure_mode_control_t   aemode;
	// bAutoExposureMode == 1 means manual mode, manual iris
	aemode.bAutoExposureMode = 1;
	if (controlSupported(aemode)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"auto exposure mode control supported");
		setCurrent(aemode);
	}

	// XXX check allowed min/max values of the exposure time
	astro::usb::uvc::exposure_time_absolute_control_t       exptime;
	if (controlSupported(exptime)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"exposure time absolute control supported");
		// get min and max time
		exptime = get(GET_MIN, exptime);
		uint32_t	minexp = exptime.dwExposureTimeAbsolute;
		exptime = get(GET_MAX, exptime);
		uint32_t	maxexp = exptime.dwExposureTimeAbsolute;
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"exposure time min = %u, max = %u", minexp, maxexp);

		// set exposure time
		exptime.dwExposureTimeAbsolute = 10000 * exposuretime;
		if ((exptime.dwExposureTimeAbsolute < minexp) ||
			(maxexp < exptime.dwExposureTimeAbsolute)) {
			debug(LOG_ERR, DEBUG_LOG, 0, "time %u out of range",
				exptime.dwExposureTimeAbsolute);
			throw std::range_error("exposure time not supported");
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "setting time %u",
				exptime.dwExposureTimeAbsolute);
			setCurrent(exptime);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure time set to %f", exposuretime);
}

/**
 * \brief disable automatic white balance
 */
void	UVCCamera::disableAutoWhiteBalance() {
	// turn of the white balance temperature control
	white_balance_temperature_auto_control_t	wbtempauto;
	wbtempauto.bWhiteBalanceTemperatureAuto = 0;
	if (controlSupported(wbtempauto)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"turn off auto white balance temperature");
		setCurrent(wbtempauto);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"white balance temperature auto control not supported");
	}

	// turn of automatic white balance component adjustmen
	white_balance_component_auto_control_t	wbcompauto;
	wbcompauto.bWhiteBalanceComponentAuto = 0;
	if (controlSupported(wbcompauto)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"turn off auto white balance components");
		setCurrent(wbcompauto);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"white balance component auto control not supported");
	}

	// set the white balance temperature
	white_balance_temperature_control_t	wbtemp;
	if (controlSupported(wbtemp)) {
		wbtemp = get(GET_DEF, wbtemp);
		setCurrent(wbtemp);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"white balance temperature set to %hu", 
			wbtemp.wWhiteBalanceTemperature);
		return;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"white balance temperature control not supported");
	}

	// if white balance temperature setting is not supported, try
	// setting components
	white_balance_component_control_t	wbcomp;
	if (controlSupported(wbcomp)) {
		wbcomp = get(GET_CUR, wbcomp);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"current White Balance components B = %hu, R = %hu", 
			wbcomp.wWhiteBalanceBlue,
			wbcomp.wWhiteBalanceRed);
		wbcomp = get(GET_DEF, wbcomp);
		wbcomp.wWhiteBalanceBlue += 10;
		setCurrent(wbcomp);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"white balance components set to B = %hu, R = %hu", 
			wbcomp.wWhiteBalanceBlue,
			wbcomp.wWhiteBalanceRed);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"white balance component control not supported");
	}
}

/**
 * \brief Set the camera gain.
 *
 * \param gain	gain value to set, default is 1
 */
void	UVCCamera::setGain(double gain) {
	astro::usb::uvc::gain_control_t	def, min, max, gaincontrol;

	if (controlSupported(gaincontrol)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "gain control is supported");
		// get the default, min and max value of the gain
		def = get(GET_DEF, astro::usb::uvc::gain_control_t());
		min = get(GET_MIN, astro::usb::uvc::gain_control_t());
		max = get(GET_MAX, astro::usb::uvc::gain_control_t());

		// what are the gain units?
		gaincontrol.wGain = gain * def.wGain;
		if ((gaincontrol.wGain > max.wGain)
			|| (gaincontrol.wGain < min.wGain)) {
			throw std::range_error("gain outside range");
		}
		setCurrent(gaincontrol);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "gain set to %f", gain);
	}
}

CameraTerminalDescriptor	*UVCCamera::cameraTerminalDescriptor() const {
	InterfaceHeaderDescriptor	*ifhd = interfaceHeaderDescriptor();
	return getPtr<CameraTerminalDescriptor>((*ifhd)[ifhd->cameraTerminalID()]);
}

ProcessingUnitDescriptor	*UVCCamera::processingUnitDescriptor() const {
	InterfaceHeaderDescriptor	*ifhd = interfaceHeaderDescriptor();
	return getPtr<ProcessingUnitDescriptor>((*ifhd)[ifhd->processingUnitID()]);
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

	// retrieve the frame descriptor
	USBDescriptorPtr	frameptr
		= getFrameDescriptor(interface, format, frame);

	// convert to a real pointer
	FrameDescriptor	*framedesc = getPtr<FrameDescriptor>(frameptr);

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

	// we now also have to find out how many bits per pixel we can
	// expect
	USBDescriptorPtr	formatptr
		= getFormatDescriptor(interface, format);
	FormatFrameBasedDescriptor	*fd
		= dynamic_cast<FormatFrameBasedDescriptor *>(&*formatptr);
	if (NULL == fd) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown pixel size");
		bitsPerPixel = 1;
	} else {
		bitsPerPixel = fd->bBitsPerPixel();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bits per pixel: %d",
			bitsPerPixel);
	}

	// just to be on the safe side, we should ask again what the
	// current settings are
	getCur(interface);
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
	getCur(interface);

	// if the frame interval is 0, we have to ask the format for
	// the default frame interval
	if (0 == frameinterval) {
		debug(LOG_WARNING, DEBUG_LOG, 0,
			"warning: no negotiated frame interval");
		frameinterval = 333333;
	}

	// compute the data rate
	double	datarate = maxvideoframesize * (10000000. / frameinterval);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "required data rate: %.1fMBps",
		datarate / 1000000.);

	// for this software, bulk transfers are preferable, if the
	// device supports them, so we check whether alt setting 0 has a
	// bulk endpoint, and return 0 in that case
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
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
	for (size_t alt = 1; alt <= interfaceptr->numAltsettings(); alt++) {
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
 * \brief Get a sequence of frames via a bulk transfer
 *
 * \param interface	interface to use for transfers
 * \param nframes	number of frames to read from the camera
 */
std::vector<FramePtr>	UVCCamera::getBulkFrames(uint8_t interface,
	unsigned int nframes) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get %d frames using bulk transfer",
		nframes);
	// find the interface on which we want to do the transfer
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	interfaceptr->claim();

	// make sure we are use alt setting 0, because that's where the
	// bulk endpoint resides
	InterfaceDescriptorPtr	ifdescptr = (*interfaceptr)[0];
	ifdescptr->altSetting();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using alt setting 0");

	// get the Endpoint for this alternate setting
	EndpointDescriptorPtr	endpoint = (*ifdescptr)[0];
	UVCBulkTransfer	transfer(endpoint, nframes, maxpayloadtransfersize,
		maxvideoframesize);

	// submit the transfer, submit will return when all the data has
	// been transferred
	device.submit(&transfer);

	// release the interface again
	try {
		interfaceptr->release();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "interface release failed: %s",
			x.what());
	}

	// convert the retrieved data to an image
	FrameFactory	ff(width, height, bitsPerPixel / 8);
	return ff(transfer.packets);
}

/**
 * \brief Get video frames using isochronous transfer
 *
 * \param interface	interface to use
 * \param nframes	number of video frames to retrieve
 */
std::vector<FramePtr>	UVCCamera::getIsoFrames(uint8_t interface,
	unsigned int nframes) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve a frame from if %d",
		interface);

	// We have to claim the interface bevor we can actually use an
	// alternate setting
	InterfacePtr	interfaceptr = (*device.activeConfig())[interface];
	interfaceptr->claim();

	// now switch to the alternate setting for that interface (this
	// succeeds if the bandwidth can be negotiated
	int	altsetting = preferredAltSetting(interface);
	InterfaceDescriptorPtr	ifdescptr = (*interfaceptr)[altsetting];
	ifdescptr->altSetting();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bandwidth negotiation complete, "
		"alt setting: %d", altsetting);

	// get the Endpoint for this alternate setting
	EndpointDescriptorPtr	endpoint = (*ifdescptr)[0];

	// now do the transfer with this alt setting, for this we first have
	// to decide for how many microframes we want to transfer anything
	UVCIsochronousTransfer	transfer(endpoint, nframes, frameinterval);

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

	// convert the retrieved data to an image
	FrameFactory	ff(width, height, bitsPerPixel / 8);
	return ff(transfer.packets);
}

/**
 * \brief
 *
 * \param interface	interface number of the video streaming interface
 */
std::vector<FramePtr>	UVCCamera::getFrames(uint8_t interface,
	unsigned int nframes) {
	// all frame retrieval goes through this method, so we use the
	// occasion to set some important variables
	getCur(interface);

	// find out what type of endpoint this interface has
	InterfacePtr	ifptr = (*device.activeConfig())[interface];
	InterfaceDescriptorPtr	ifdptr = (*ifptr)[0];
	if (ifdptr->numEndpoints() > 0) {
		EndpointDescriptorPtr	endpoint = (*ifdptr)[0];
		if (endpoint->isBulk()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "using bulk endpoint");
			return getBulkFrames(interface, nframes);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "using isochronous endpoint");
	return getIsoFrames(interface, nframes);
}

/**
 * \brief Get a single frame.
 *
 * \param ifno interface number
 */
FramePtr	UVCCamera::getFrame(uint8_t ifno) {
	std::vector<FramePtr>	frames = getFrames(ifno, 1);
	if (frames.size() == 0) {
		throw std::length_error("no frames returned by getFrames");
	}
	return *frames.begin();
}

} // namespace uvc
} // namespace usb
} // namespace astro
