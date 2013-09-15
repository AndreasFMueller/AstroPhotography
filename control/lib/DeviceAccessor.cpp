/*
 * DeviceAccessor.cpp -- simplify access to devices
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevaccess.h>
#include <AstroLoader.h>

namespace astro {
namespace device {

DeviceLocatorPtr	DeviceAccessorBase::locator(const std::string& name) {
	astro::module::ModulePtr	module = _repository.getModule(name);
	return module->getDeviceLocator();
}

template<>
astro::camera::CameraPtr
	DeviceAccessor<astro::camera::CameraPtr>::get(
		const DeviceName& name) {
	return locator(name.modulename())->getCamera((std::string)name);
}

template<>
astro::camera::GuiderPortPtr
	DeviceAccessor<astro::camera::GuiderPortPtr>::get(
		const DeviceName& name) {
	return locator(name.modulename())->getGuiderPort((std::string)name);
}

template<>
astro::camera::FilterWheelPtr
	DeviceAccessor<astro::camera::FilterWheelPtr>::get(
		const DeviceName& name) {
	return locator(name.modulename())->getFilterWheel((std::string)name);
}

template<>
astro::camera::CoolerPtr
	DeviceAccessor<astro::camera::CoolerPtr>::get(
		const DeviceName& name) {
	return locator(name.modulename())->getCooler((std::string)name);
}

template<>
astro::camera::FocuserPtr
	DeviceAccessor<astro::camera::FocuserPtr>::get(
		const DeviceName& name) {
	return locator(name.modulename())->getFocuser((std::string)name);
}

} // namespace device
} // namespace astro
