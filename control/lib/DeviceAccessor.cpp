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

DeviceLocatorPtr	DeviceAccessorBase::locator(const std::string& name) {
	astro::module::ModulePtr	module = _repository.getModule(name);
	return module->getDeviceLocator();
}

template<>
astro::camera::CameraPtr
	DeviceAccessor<astro::camera::CameraPtr>::get(
		const DeviceName& name) {
	if (name.type() != DeviceName::Camera) {
		std::string	msg = stringprintf("%s is not a camera name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
		throw BadParameter(msg);
	}
	return locator(name.modulename())->getCamera((std::string)name);
}

template<>
astro::camera::CcdPtr
	DeviceAccessor<astro::camera::CcdPtr>::get(
		const DeviceName& name) {
	if (name.type() != DeviceName::Ccd) {
		std::string	msg = stringprintf("%s is not a ccd name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
		throw BadParameter(msg);
	}
	return locator(name.modulename())->getCcd((std::string)name);
}

template<>
astro::camera::GuiderPortPtr
	DeviceAccessor<astro::camera::GuiderPortPtr>::get(
		const DeviceName& name) {
	if (name.type() != DeviceName::Guiderport) {
		std::string	msg = stringprintf("%s is not a guiderport name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
		throw BadParameter(msg);
	}
	return locator(name.modulename())->getGuiderPort((std::string)name);
}

template<>
astro::camera::FilterWheelPtr
	DeviceAccessor<astro::camera::FilterWheelPtr>::get(
		const DeviceName& name) {
	if (name.type() != DeviceName::Filterwheel) {
		std::string	msg = stringprintf("%s is not a filterwheel name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
		throw BadParameter(msg);
	}
	return locator(name.modulename())->getFilterWheel((std::string)name);
}

template<>
astro::camera::CoolerPtr
	DeviceAccessor<astro::camera::CoolerPtr>::get(
		const DeviceName& name) {
	if (name.type() != DeviceName::Cooler) {
		std::string	msg = stringprintf("%s is not a cooler name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
		throw BadParameter(msg);
	}
	return locator(name.modulename())->getCooler((std::string)name);
}

template<>
astro::camera::FocuserPtr
	DeviceAccessor<astro::camera::FocuserPtr>::get(
		const DeviceName& name) {
	if (name.type() != DeviceName::Focuser) {
		std::string	msg = stringprintf("%s is not a focuser name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "bad request: %s", msg.c_str());
		throw BadParameter(msg);
	}
	return locator(name.modulename())->getFocuser((std::string)name);
}

} // namespace device
} // namespace astro
