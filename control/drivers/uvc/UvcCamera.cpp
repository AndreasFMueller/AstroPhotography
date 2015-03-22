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

/**
 * \brief Auxiliary function to generate the camera name from the deviceptr
 */
static astro::DeviceName        cameraname(DevicePtr& deviceptr) {
	DeviceName	modulename("module:uvc");
	return DeviceName(modulename, DeviceName::Camera,
			deviceptr->getDeviceName());
}

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
	// note that it is apparently impossible to determine the pixel
	// size of a UVC camera. However, for a guide camera it is essential
	// to know the CCD size. Lacking any better method, we set the
	// pixel size to 5 microns which is probably the right order of
	// magnitude for any camera useful for astronomy purposes (smaller
	// chips will not be sensitive enough for guiding), but a littel too
	// small in most cases. That isn't too serious, because it just
	// means that calibration algorithm will be a bit more careful
	// not to move to telescope during calibration.
	astro::image::ImageSize	ccdsize
		= astro::image::ImageSize(framedescriptor->wWidth(),
			framedescriptor->wHeight());
	std::string	ccdname = stringprintf("%dx%d:%d:%d:%d:%s",
		ccdsize.width(), ccdsize.height(),
		uvcccd.interface, uvcccd.format, uvcccd.frame,
		uvcccd.guid.c_str());
	DeviceName	devname(name(), DeviceName::Ccd, ccdname);
	CcdInfo	ccd(devname, ccdsize, ccds.size() - 1);
	ccd.pixelwidth(0.000005); // fake pixel size, as it is not available
	ccd.pixelheight(0.000005); // for a UVC camera
	ccd.addMode(Binning(1,1));
	ccdinfo.push_back(ccd);

	// add ccdinfo
	std::string	n = devname;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding CCD %s", n.c_str());
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

UvcCamera::UvcCamera(DevicePtr& _deviceptr) : Camera(cameraname(deviceptr)),
	deviceptr(_deviceptr), camera(*deviceptr, true) {
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

CcdPtr	UvcCamera::getCcd0(size_t ccdindex) {
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
