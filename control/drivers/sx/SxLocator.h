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

/**
 * \brief The Locator class for Starlight Express devices
 *
 * All Starlight Express devices are USB devices, so this locator is 
 * essentially a wrapper around a USB context which serves as a factory
 * for the Starlight Express USB devices.
 */
class SxCameraLocator : public DeviceLocator {
	Context	context;
public:
	SxCameraLocator();
	virtual ~SxCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxLocator_h */
