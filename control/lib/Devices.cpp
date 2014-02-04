/*
 * Devices.cpp -- device access implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroLoader.h>
#include <AstroFormat.h>
#include <AstroDevaccess.h>
#include <AstroDebug.h>

using namespace astro::camera;
using namespace astro::device;

namespace astro {
namespace module {

/**
 * \brief construct a list of available devices of a given type
 */
Devices::devicelist	Devices::getDevicelist(DeviceName::device_type type) {
	Devices::devicelist	result;

	// get a list of driver modules
	std::vector<std::string>	modulenames = _repository.moduleNames();

	// go through the list
	std::vector<std::string>::const_iterator	i;
	for (i = modulenames.begin(); i != modulenames.end(); i++) {
		// get the associated module
		std::string	modulename = *i;
		ModulePtr	module = _repository.getModule(modulename);

		// get the descriptor
		ModuleDescriptorPtr	descriptor = module->getDescriptor();
		if (!descriptor->hasDeviceLocator()) {
			continue;
		}

		// get the device locator
		DeviceLocatorPtr	locator = module->getDeviceLocator();

		// get a list of devices of a given type
		std::vector<DeviceName>	l = locator->getDeviceList(type);
		std::copy(l.begin(), l.end(), back_inserter(result));
	}
	
	// return the aggregated list
	return result;
}

/**
 * \brief Get a camera by name
 */
CameraPtr	Devices::getCamera(const DeviceName& name) {
	return DeviceAccessor<CameraPtr>(_repository).get(name);
}

/**
 * \brief Get a ccd by name
 */
CcdPtr	Devices::getCcd(const DeviceName& name) {
	return DeviceAccessor<CcdPtr>(_repository).get(name);
}

/**
 * \brief Get a cooler by name
 */
CoolerPtr	Devices::getCooler(const DeviceName& name) {
	return DeviceAccessor<CoolerPtr>(_repository).get(name);
}

/**
 * \brief Get a filter wheel by name
 */
FilterWheelPtr	Devices::getFilterWheel(const DeviceName& name) {
	return DeviceAccessor<FilterWheelPtr>(_repository).get(name);
}

/**
 * \brief Get a focuser by name
 */
FocuserPtr	Devices::getFocuser(const DeviceName& name) {
	return DeviceAccessor<FocuserPtr>(_repository).get(name);
}

/**
 * \brief Get a guider port by name
 */
GuiderPortPtr	Devices::getGuiderPort(const DeviceName& name) {
	return DeviceAccessor<GuiderPortPtr>(_repository).get(name);
}

} // namespace module
} // namespace astro
