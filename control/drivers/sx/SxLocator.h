/*
 * SxLocator.h -- class to search for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxLocator_h
#define _SxLocator_h

#include <AstroDevice.h>
#include <AstroCamera.h>
#include <AstroUSB.h>

using namespace astro::usb;
using namespace astro::device;

namespace astro {
namespace camera {
namespace sx {

class SxCameraLocator : public DeviceLocator {
	Context	context;
public:
	SxCameraLocator();
	virtual ~SxCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(DeviceLocator::device_type device = DeviceLocator::CAMERA);
	virtual CameraPtr	getCamera(const std::string& name);
	virtual CameraPtr	getCamera(size_t index);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxLocator_h */
