/*
 * UvcLocator.h -- declarations of the camera 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _UvcLocator_h
#define _UvcLocator_h

#include <AstroUSB.h>
#include <AstroLocator.h>
#include <AstroCamera.h>

using namespace astro::usb;
using namespace astro::camera;
using namespace astro::device;

namespace astro {
namespace camera {
namespace uvc {

/**
 * \brief The UVC camera locator
 *
 * Each UVC camera is also a camera from the point of view of this 
 */
class UvcCameraLocator : public DeviceLocator {
	Context	context;
public:
	UvcCameraLocator();
	virtual ~UvcCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	CameraPtr	getCamera0(const DeviceName& name);
};

} // namespace uvc
} // namespace camera
} // namespace uvc

#endif /* _UvcLocator_h */
