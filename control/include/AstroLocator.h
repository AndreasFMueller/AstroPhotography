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

/**
 * \brief The device locator can locate device within a module
 *
 * The device locator keeps a cache of all devices ever retrieved from this
 * locator, which ensures that 
 */
class   DeviceLocator {
	std::map<std::string, astro::camera::CameraPtr>		cameracache;
	std::map<std::string, astro::camera::GuiderPortPtr>	guiderportcache;
	std::map<std::string, astro::camera::FilterWheelPtr>	filterwheelcache;
	std::map<std::string, astro::camera::CoolerPtr>		coolercache;
	std::map<std::string, astro::camera::FocuserPtr>	focusercache;
protected:
	virtual	astro::camera::CameraPtr	getCamera0(const std::string& name);
	virtual	astro::camera::GuiderPortPtr	getGuiderPort0(const std::string& name);
	virtual	astro::camera::FilterWheelPtr	getFilterWheel0(const std::string& name);
	virtual	astro::camera::CoolerPtr	getCooler0(const std::string& name);
	virtual	astro::camera::FocuserPtr	getFocuser0(const std::string& name);
public:
	DeviceLocator();
	virtual ~DeviceLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	typedef enum device_type {
		CAMERA, FOCUSER, GUIDERPORT, FILTERWHEEL, COOLER
	} device_type;
	virtual std::vector<std::string>	getDevicelist(
		device_type device = CAMERA);
	astro::camera::CameraPtr	getCamera(const std::string& name);
	astro::camera::CameraPtr	getCamera(size_t index);
	astro::camera::GuiderPortPtr	getGuiderPort(const std::string& name);
	astro::camera::FilterWheelPtr	getFilterWheel(const std::string& name);
	astro::camera::CoolerPtr	getCooler(const std::string& name);
	astro::camera::FocuserPtr	getFocuser(const std::string& name);
};

typedef std::shared_ptr<DeviceLocator>	DeviceLocatorPtr;

//////////////////////////////////////////////////////////////////////
// adapter class for DeviceLocator to extract objects of a given type
//////////////////////////////////////////////////////////////////////
template<typename device>
class LocatorAdapter {
        astro::device::DeviceLocatorPtr _locator;
public:
        LocatorAdapter(astro::device::DeviceLocatorPtr locator)
                : _locator(locator) { }
        typename device::sharedptr      get(const std::string& name);
};

template<>
astro::camera::CameraPtr       LocatorAdapter<astro::camera::Camera>::get(
					const std::string& name);

template<>
astro::camera::GuiderPortPtr   LocatorAdapter<astro::camera::GuiderPort>::get(
					const std::string& name);

template<>
astro::camera::FilterWheelPtr   LocatorAdapter<astro::camera::FilterWheel>::get(
					const std::string& name);

template<>
astro::camera::CoolerPtr        LocatorAdapter<astro::camera::Cooler>::get(
					const std::string& name);

template<>
astro::camera::FocuserPtr       LocatorAdapter<astro::camera::Focuser>::get(
					const std::string& name);

} // namespace device
} // namespace astro

#endif /* _AstroLocator_h */
