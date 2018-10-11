/*
 * DeviceServantLocator.cpp -- device servant locator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <DeviceServantLocator.h>
#include <AstroDebug.h>
#include <CameraI.h>
#include <CcdI.h>
#include <CoolerI.h>
#include <FilterWheelI.h>
#include <FocuserI.h>
#include <GuidePortI.h>
#include <AdaptiveOpticsI.h>
#include <MountI.h>
#include <AstroDevice.h>
#include <AstroDevaccess.h>
#include <NameConverter.h>
#include <AstroFormat.h>
#include <ImageDirectory.h>

using namespace astro::device;
using namespace astro::camera;

namespace snowstar {

/**
 * \brief Create the locator for device servants
 */
DeviceServantLocator::DeviceServantLocator(
	astro::module::ModuleRepositoryPtr repository)
	: _repository(repository) {
	if (!repository) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no repository!");
	}
}

/**
 * \brief Create a servant 
 */
Ice::ObjectPtr	DeviceServantLocator::locate(const Ice::Current& current,
			Ice::LocalObjectPtr& /* cookie */) {
	std::string	name = NameConverter::urldecode(current.id.name);

	// the device we are going to return
	Ice::ObjectPtr	ptr;

	// look in the cache
	devicemap::iterator	i = devices.find(name);
	if (i != devices.end()) {
		ptr = i->second;
		return ptr;
	}

	// log the request
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get new device servant for name %s",
		name.c_str());

	// convert the name into a devicename
	astro::DeviceName	devicename(name);

	// depending on the device type, use a suitable accessor to
	// get the device
	switch (devicename.type()) {
	case astro::DeviceName::AdaptiveOptics:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting an AO unit");
		ptr = new AdaptiveOpticsI(
			DeviceAccessor<astro::camera::AdaptiveOpticsPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Camera:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a camera");
		ptr = new CameraI(
			DeviceAccessor<astro::camera::CameraPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Ccd:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a CCD");
		ptr = new CcdI(
			DeviceAccessor<astro::camera::CcdPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Cooler:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a Cooler");
		ptr = new CoolerI(
			DeviceAccessor<astro::camera::CoolerPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Filterwheel:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a Filterwheel");
		ptr = new FilterWheelI(
			DeviceAccessor<astro::camera::FilterWheelPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Focuser:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a Focuser");
		ptr = new FocuserI(
			DeviceAccessor<astro::camera::FocuserPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Guideport:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a Guideport");
		ptr = new GuidePortI(
			DeviceAccessor<astro::camera::GuidePortPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Module:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a module");
		throw NotImplemented("no module access through devices");

	case astro::DeviceName::Mount:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting a mount");
		ptr = new MountI(
			DeviceAccessor<astro::device::MountPtr>(
				_repository).get(devicename));
		break;
	};

	// handle the case where we have no servant for the device
	if (!ptr) {
		debug(LOG_ERR, DEBUG_LOG, 0, "device %s not found",
			name.c_str());
		return ptr;
	}

	// inform the debugger that we have in fact found a matching device
	debug(LOG_DEBUG, DEBUG_LOG, 0, "have found device for %s",
		name.c_str());
	devices.insert(std::make_pair(name, ptr));
	return ptr;
}

void	DeviceServantLocator::finished(const Ice::Current& /* current */,
		const Ice::ObjectPtr& /* servant */,
		const Ice::LocalObjectPtr& /* cookie */) {
}

void	DeviceServantLocator::deactivate(const std::string& /* category */) {
}

} // namespace snowstar
