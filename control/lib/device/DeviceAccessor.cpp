/*
 * DeviceAccessor.cpp -- simplify access to devices
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevaccess.h>
#include <AstroLoader.h>
#include <AstroFormat.h>
#include <AstroExceptions.h>

namespace astro {
namespace device {

//////////////////////////////////////////////////////////////////////
// DeviceAccessorBase implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Find the best accessible name of a device
 *
 * If the current process serves a network accessible device, then it does
 * not make sense to access it through the network protocol. Instead, 
 * the accessor should return a direct instance of the object. This
 * method returns the name that should be accessed in such cases.
 *
 * \param name	Device name to convert to the most accessible one
 */
DeviceName	DeviceAccessorBase::accessible(const DeviceName& name) const {
	if (name.isLocalDevice()) {
		return name;
	}
	if (name.isServedByUs()) {
		return name.localdevice();
	}
	return name;
}

/**
 * \brief Retrieve the device locator for the name
 */
DeviceLocatorPtr	DeviceAccessorBase::locator(const std::string& name) {
	astro::module::ModulePtr	module = _repository.getModule(name);
	return module->getDeviceLocator();
}

/**
 * \brief Check that a device name is of the right type
 */
void	DeviceAccessorBase::check(const DeviceName& name,
		DeviceName::device_type type) const {
	if (name.hasType(type)) {
		return;
	}
	std::string	msg = stringprintf("%s is not a %s name",
				name.toString().c_str(),
				DeviceName::type2string(type).c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
	throw BadParameter(msg);
}

//////////////////////////////////////////////////////////////////////
// DeviceAccessor implementation
//////////////////////////////////////////////////////////////////////

template<>
astro::camera::CameraPtr
	DeviceAccessor<astro::camera::CameraPtr>::get(const DeviceName& name) {
	check(name, DeviceName::Camera);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getCamera((std::string)a);
}

template<>
astro::camera::CcdPtr
	DeviceAccessor<astro::camera::CcdPtr>::get(const DeviceName& name) {
	check(name, DeviceName::Ccd);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getCcd((std::string)a);
}

template<>
astro::camera::GuidePortPtr
	DeviceAccessor<astro::camera::GuidePortPtr>::get(
		const DeviceName& name) {
	check(name, DeviceName::Guideport);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getGuidePort((std::string)a);
}

template<>
astro::camera::FilterWheelPtr
	DeviceAccessor<astro::camera::FilterWheelPtr>::get(
		const DeviceName& name) {
	check(name, DeviceName::Filterwheel);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getFilterWheel((std::string)a);
}

template<>
astro::camera::CoolerPtr
	DeviceAccessor<astro::camera::CoolerPtr>::get(
		const DeviceName& name) {
	check(name, DeviceName::Cooler);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getCooler((std::string)a);
}

template<>
astro::camera::FocuserPtr
	DeviceAccessor<astro::camera::FocuserPtr>::get(
		const DeviceName& name) {
	check(name, DeviceName::Focuser);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getFocuser((std::string)a);
}

template<>
astro::camera::AdaptiveOpticsPtr
	DeviceAccessor<astro::camera::AdaptiveOpticsPtr>::get(
		const DeviceName& name) {
	check(name, DeviceName::AdaptiveOptics);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getAdaptiveOptics((std::string)a);
}

template<>
astro::device::MountPtr
	DeviceAccessor<astro::device::MountPtr>::get(
		const DeviceName& name) {
	check(name, DeviceName::Mount);
	DeviceName	a = accessible(name);
	return locator(a.modulename())->getMount((std::string)a);
}

} // namespace device
} // namespace astro
