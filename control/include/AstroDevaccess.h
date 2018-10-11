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
	astro::module::ModuleRepositoryPtr	_repository;
protected:
	DeviceName	accessible(const DeviceName& name) const;
	void	check(const DeviceName& name,
			DeviceName::device_type type) const;
public:
	DeviceAccessorBase(astro::module::ModuleRepositoryPtr repository);
	DeviceLocatorPtr	locator(const std::string& modulename);
	DeviceLocatorPtr	locator(const DeviceName& devicename) {
		return locator(devicename.modulename());
	}
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
	DeviceAccessor(astro::module::ModuleRepositoryPtr repository)
		: DeviceAccessorBase(repository) { }
	DeviceType	get(const DeviceName& /* name */) {
		throw std::logic_error("DeviceAccessor::get not specialized "
			"for this type");
	}
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
astro::camera::GuidePortPtr
	DeviceAccessor<astro::camera::GuidePortPtr>::get(
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

template<>
astro::camera::AdaptiveOpticsPtr
	DeviceAccessor<astro::camera::AdaptiveOpticsPtr>::get(
		const DeviceName& name);

template<>
astro::device::MountPtr
	DeviceAccessor<astro::device::MountPtr>::get(
		const DeviceName& name);

} // namespace device
} // namespace astro

#endif /* _AstroDevAccess_h */
