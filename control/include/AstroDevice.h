/*
 * AstroDevice.h -- Device manager
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDevice_h
#define _AstroDevice_h

#include <AstroCamera.h>

namespace astro {
namespace device {

class   DeviceLocator {
public:
	DeviceLocator();
	virtual ~DeviceLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	typedef enum device_type { CAMERA, FOCUSER, GUIDERPORT } device_type;
	virtual std::vector<std::string>	getDevicelist(
		device_type device = CAMERA);
	virtual astro::camera::CameraPtr	getCamera(const std::string& name);
	virtual astro::camera::GuiderPortPtr	getGuiderPort(const std::string& name);
};
typedef std::tr1::shared_ptr<DeviceLocator>	DeviceLocatorPtr;

} // namespace device
} // namespace astro

#endif /* _AstroDevice_h */
