/*
 * UvcCamera.cpp -- UVC Camera implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <UvcCamera.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <UvcCcd.h>
#include <UvcUtils.h>

namespace astro {
namespace camera {
namespace uvc {

using astro::usb::uvc::HeaderDescriptor;
using astro::usb::uvc::FormatFrameBasedDescriptor;

void	UvcCamera::addFrame(int interface, int format, int frame,
	const std::string& guid, FrameDescriptor *framedescriptor) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d, format %d, frame %d",
		interface, format, frame);

	// UVC interface/format/frame information
	uvcccd_t	uvcccd;
	uvcccd.interface = interface;
	uvcccd.format = format;
	uvcccd.frame = frame;
	uvcccd.guid = guid;
	ccds.push_back(uvcccd);

	// standard CcdInfo
	astro::image::ImageSize	ccdsize
		= astro::image::ImageSize(framedescriptor->wWidth(),
			framedescriptor->wHeight());
	std::string	ccdname = stringprintf("%dx%d/%d/%d/%d/%s",
		ccdsize.width(), ccdsize.height(),
		uvcccd.interface, uvcccd.format, uvcccd.frame,
		uvcccd.guid.c_str());
	CcdInfo	ccd(ccdname, ccdsize, ccds.size() - 1);
	ccd.addMode(Binning(1,1));
	ccdinfo.push_back(ccd);

	// add ccdinfo
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding CCD %s", ccd.name().c_str());
}

void	UvcCamera::addFormat(int interface, int format,
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
	std::string	guid("(unknown)");
	switch (subtype) {
	case VS_FORMAT_UNCOMPRESSED:
	case VS_FORMAT_FRAME_BASED:
		guid = dynamic_cast<FormatFrameBasedDescriptor *>(
			formatdescriptor)->guidFormat();
		break;
	default:
		return;
	}

	// if we get to this point, we know that we are working on a format
	// descriptor that we understand

	// we can add all the frames
	int	framecount = formatdescriptor->numFrames();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "frames: %d", framecount);
	for (int frameindex = 1; frameindex <= framecount; frameindex++) {
		FrameDescriptor	*framedescriptor
			= getPtr<FrameDescriptor>((*formatdescriptor)[frameindex - 1]);
		addFrame(interface, format, frameindex, guid, framedescriptor);
	}
}

void	UvcCamera::addHeader(int interface, HeaderDescriptor *headerdescriptor) {
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

UvcCamera::UvcCamera(DevicePtr& _deviceptr) : deviceptr(_deviceptr),
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
	for (int ifno = firstinterface + 1; ifno < lastinterface; ifno++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "interface %d", ifno);
		USBDescriptorPtr	header
			= camera.getHeaderDescriptor(ifno);
		// find out how many formats this header contains
		HeaderDescriptor	*hd = getPtr<HeaderDescriptor>(header);
		addHeader(ifno, hd);
	}
}

UvcCamera::~UvcCamera() {
}

CcdPtr	UvcCamera::getCcd(const std::string& name) {
	// locate the camera using the name
	return CcdPtr();
}

CcdPtr	UvcCamera::getCcd(size_t ccdindex) {
	uvcccd_t	uc = ccds[ccdindex];
	CcdInfo	info = ccdinfo[ccdindex];
	UvcCcd	*uvcccd = NULL;
	if (uc.guid == std::string("YUY2")) {
		uvcccd = new UvcCcdYUY2(info, uc.interface, uc.format, uc.frame,
				*this);
	}
	if (uc.guid == std::string("Y800")) {
		uvcccd = new UvcCcdY800(info, uc.interface, uc.format, uc.frame,
				*this);
	}
	if (uc.guid == std::string("BY8 ")) {
		uvcccd = new UvcCcdBY8(info, uc.interface, uc.format, uc.frame,
				*this);
	}
	if (NULL == uvcccd) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no known frame format: %s",
			uc.guid.c_str());
		throw std::runtime_error("unknown frame format");
	}

	return CcdPtr(uvcccd);
}

void	UvcCamera::selectFormatAndFrame(int interface, int format, int frame) {
	try {
		camera.selectFormatAndFrame(interface, format, frame);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot select interface %d, "
			"format %d, frame %d: %s",
			interface, format, frame, x.what());
		throw UvcError("cannot set format/frame");
	}
}

void	UvcCamera::setExposureTime(double exposuretime) {
	try {
		camera.setExposureTime(exposuretime);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot set exposure time: %s",
			x.what());
		throw UvcError("cannot set exposure time");
	}
}

bool	UvcCamera::hasGain() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking whether camera has gain");
	return camera.hasGain();
}

void	UvcCamera::setGain(double gain) {
	try {
		camera.setGain(gain);
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot set gain: %s",
			x.what());
		throw UvcError("cannot set gain");
	}
}

std::pair<float, float>	UvcCamera::getGainInterval() {
	try {
		return camera.getGainInterval();
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get gain interval: %s",
			x.what());
		throw UvcError("cannot get gain interval");
	}
}

void	UvcCamera::disableAutoWhiteBalance() {
	try {
		camera.disableAutoWhiteBalance();
	} catch (std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot disable WB: %s",
			x.what());
	}
}

std::vector<FramePtr>	UvcCamera::getFrames(int interface,
	unsigned int nframes) {
	return camera.getFrames(interface, nframes);
}

} // namespace uvc
} // namespace camera
} // namespace astro
