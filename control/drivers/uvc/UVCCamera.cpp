/*
 * UVCCamera.cpp -- UVC Camera implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UVCCamera.h>
#include <debug.h>
#include <Format.h>

namespace astro {
namespace camera {
namespace uvc {

using astro::usb::uvc::HeaderDescriptor;

void	UVCCamera::addFrame(int interface, int format, int frame,
	FrameDescriptor *framedescriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d, format %d, frame %d",
		interface, format, frame);
	uvcccd_t	ccd;
	ccd.interface = interface;
	ccd.format = format;
	ccd.frame = frame;
	ccd.width = framedescriptor->wWidth();
	ccd.height = framedescriptor->wHeight();
	ccd.name = stringprintf("%dx%d/%d/%d/%d", ccd.width, ccd.height,
		ccd.interface, ccd.format, ccd.frame);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding CCD %s", ccd.name.c_str());
	ccds.push_back(ccd);
}

void	UVCCamera::addFormat(int interface, int format,
		FormatDescriptor *formatdescriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d, format %d",
		interface, format);
	// descriptor type must be an interface specific descriptor
	int	type = formatdescriptor->bDescriptorType();
	if (type != CS_INTERFACE) {
		return;
	}
	// subtype must be uncompressed or frame based
	int	subtype = formatdescriptor->bDescriptorSubtype();
	switch (subtype) {
	case VS_FORMAT_UNCOMPRESSED:
	case VS_FORMAT_FRAME_BASED:
		break;
	default:
		return;
	}
	// if we get to this point, then we can add all the frames
	int	framecount = formatdescriptor->numFrames();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "frames: %d", framecount);
	for (int frameindex = 1; frameindex <= framecount; frameindex++) {
		FrameDescriptor	*framedescriptor
			= getPtr<FrameDescriptor>((*formatdescriptor)[frameindex - 1]);
		addFrame(interface, format, frameindex, framedescriptor);
	}
}

void	UVCCamera::addHeader(int interface, HeaderDescriptor *headerdescriptor) {
	int	formatcount = headerdescriptor->bNumFormats();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d formats", formatcount);

	for (int findex = 1; findex <= formatcount; findex++) {
		USBDescriptorPtr	formatptr
			= camera.getFormatDescriptor(interface, findex);
		FormatDescriptor	*formatdescriptor
			= getPtr<FormatDescriptor>(formatptr);
		addFormat(interface, findex, formatdescriptor);
	}
}

UVCCamera::UVCCamera(DevicePtr& _deviceptr) : deviceptr(_deviceptr),
	camera(*deviceptr, true) {
	// show what we have in this camera
	std::cout << camera;

	// find out how many different formats this camera has, we are
	// only interested in frames that are uncompressed or frame based,
	// all other types are not acceptable
	int	firstinterface = camera.iad().bFirstInterface();
	int	interfacecount = camera.iad().bInterfaceCount();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "streaming interfaces: %d",
		interfacecount - 1);
	int	lastinterface = firstinterface + interfacecount;

	int	n = camera.numberVideoStreamingInterfaces();
	for (int ifno = firstinterface + 1; ifno < lastinterface; ifno++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d", ifno);
		USBDescriptorPtr	header
			= camera.getHeaderDescriptor(ifno);
		// find out how many formats this header contains
		HeaderDescriptor	*hd = getPtr<HeaderDescriptor>(header);
		addHeader(ifno, hd);
	}

	// now we know all the format/frame combinations, they are contained
	// in the array
	numberCcds = ccds.size();
}

UVCCamera::~UVCCamera() {
}

CcdPtr	UVCCamera::getCcd(int ccdindex) {
	return CcdPtr();
}

} // namespace uvc
} // namespace camera
} // namespace astro
