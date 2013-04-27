/*
 * UVCCamera.h -- USB Video Class camera
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _UVCCamera_h
#define _UVCCamera_h

#include <AstroUVC.h>
#include <AstroCamera.h>

using namespace astro::usb;
using astro::usb::uvc::FrameDescriptor;
using astro::usb::uvc::FormatDescriptor;
using astro::usb::uvc::HeaderDescriptor;

namespace astro {
namespace camera {
namespace uvc {

class UVCCamera : public Camera {
	DevicePtr	deviceptr;
	astro::usb::uvc::UVCCamera	camera;
	typedef struct uvcccd_s {
		int	interface;
		int	format;
		int	frame;
		int	width;
		int	height;
		std::string	name;
	} uvcccd_t;

	std::vector<uvcccd_t>	ccds;

private:
	// auxiliary functions needed to build the CCD list
	void    addFrame(int interface, int format, int frame, 
			FrameDescriptor *framedescriptor);
	void    addFormat(int interface, int format, 
			FormatDescriptor *formatdescriptor);
	void    addHeader(int interface,
			HeaderDescriptor *headerdescriptor);

public:
	UVCCamera(DevicePtr& deviceptr);
	virtual ~UVCCamera();
	CcdPtr	getCcd(int ccdindex);
};

} // namespace uvc
} // namespace camera
} // namespace astro

#endif /* _UVCCamera_h */
