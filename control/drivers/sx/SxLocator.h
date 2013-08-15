/*
 * SxLocator.h -- class to search for starlight express cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxLocator_h
#define _SxLocator_h

#include <AstroLocator.h>
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
protected:
	virtual CameraPtr	getCamera0(const std::string& name);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxLocator_h */
