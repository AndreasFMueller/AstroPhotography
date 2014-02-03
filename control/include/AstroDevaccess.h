/*
 * AstroDevaccess.h -- simplified device access in the repository
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDevaccess_h
#define _AstroDevaccess_h

#include <AstroLoader.h>
#include <AstroLocator.h>
#include <AstroTypes.h>

namespace astro {
namespace device {

/**
 * \brief Device accessor to simplify device access
 *
 * Accessing a device through the repsitory/module/locator chain is somewhat
 * tedious. This class simplfies the access, as it allows to directly specify
 * the type of object one would like to retrieve. The individual types of
 * objects that can be retreived are implemented as templates derived from
 * this base class, which essentially implements the access to the device
 * locator of a module
 */
class DeviceAccessorBase {
	astro::module::Repository&	_repository;
public:
	DeviceAccessorBase(astro::module::Repository& repository)
		: _repository(repository) { }
	DeviceLocatorPtr	locator(const std::string& modulename);
};

/**
 * \brief Accessor template for a device of type Device
 *
 * Using the locator provided by the base class, this template access a
 * device of the chosen type.
 */
template<typename DeviceType>
class DeviceAccessor : DeviceAccessorBase {
public:
	DeviceAccessor(astro::module::Repository& repository)
		: DeviceAccessorBase(repository) { }
	DeviceType	get(const DeviceName& name) { return DeviceType(); }
};

template<>
astro::camera::CameraPtr
	DeviceAccessor<astro::camera::CameraPtr>::get(
		const DeviceName& name);

template<>
astro::camera::CcdPtr
	DeviceAccessor<astro::camera::CcdPtr>::get(
		const DeviceName& name);

template<>
astro::camera::GuiderPortPtr
	DeviceAccessor<astro::camera::GuiderPortPtr>::get(
		const DeviceName& name);

template<>
astro::camera::FilterWheelPtr
	DeviceAccessor<astro::camera::FilterWheelPtr>::get(
		const DeviceName& name);

template<>
astro::camera::CoolerPtr
	DeviceAccessor<astro::camera::CoolerPtr>::get(
		const DeviceName& name);

template<>
astro::camera::FocuserPtr
	DeviceAccessor<astro::camera::FocuserPtr>::get(
		const DeviceName& name);

} // namespace device
} // namespace astro

#endif /* _AstroDevAccess_h */
