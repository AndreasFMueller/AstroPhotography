/*
 * AstroLocator.h -- DeviceLocator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroLocator_h
#define _AstroLocator_h

#include <AstroCamera.h>
#include <map>

namespace astro {
namespace device {

class   DeviceLocator {
	std::map<std::string, astro::camera::CameraPtr>	cameracache;
	std::map<std::string, astro::camera::GuiderPortPtr>	guiderportcache;
	std::map<std::string, astro::camera::FilterWheelPtr>	filterwheelcache;
	std::map<std::string, astro::camera::CoolerPtr>	coolercache;
protected:
	virtual	astro::camera::CameraPtr	getCamera0(const std::string& name);
	virtual	astro::camera::GuiderPortPtr	getGuiderPort0(const std::string& name);
	virtual	astro::camera::FilterWheelPtr	getFilterWheel0(const std::string& name);
	virtual	astro::camera::CoolerPtr	getCooler0(const std::string& name);
public:
	DeviceLocator();
	virtual ~DeviceLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	typedef enum device_type { CAMERA, FOCUSER, GUIDERPORT, FILTERWHEEL, COOLER } device_type;
	virtual std::vector<std::string>	getDevicelist(
		device_type device = CAMERA);
	astro::camera::CameraPtr	getCamera(const std::string& name);
	astro::camera::CameraPtr	getCamera(size_t index);
	astro::camera::GuiderPortPtr	getGuiderPort(const std::string& name);
	astro::camera::FilterWheelPtr	getFilterWheel(const std::string& name);
	astro::camera::CoolerPtr	getCooler(const std::string& name);
};

typedef std::tr1::shared_ptr<DeviceLocator>	DeviceLocatorPtr;

} // namespace device
} // namespace astro

#endif /* _AstroLocator_h */
