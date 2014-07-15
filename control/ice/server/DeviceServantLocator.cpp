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
#include <GuiderPortI.h>
#include <AstroDevice.h>
#include <AstroDevaccess.h>
#include <NameConverter.h>

using namespace astro::device;
using namespace astro::camera;

namespace snowstar {

/**
 * \brief Create the locator for device servants
 */
DeviceServantLocator::DeviceServantLocator(
	astro::module::Repository& repository,
	astro::image::ImageDirectory& imagedirectory)
	: _repository(repository), _imagedirectory(imagedirectory) {
}

/**
 * \brief Create a servant 
 */
Ice::ObjectPtr	DeviceServantLocator::locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie) {
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
		throw NotImplemented("adaptive optics not implemented");

	case astro::DeviceName::Camera:
		ptr = new CameraI(
			DeviceAccessor<astro::camera::CameraPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Ccd:
		ptr = new CcdI(
			DeviceAccessor<astro::camera::CcdPtr>(
				_repository).get(devicename),
			_imagedirectory);
		break;

	case astro::DeviceName::Cooler:
		ptr = new CoolerI(
			DeviceAccessor<astro::camera::CoolerPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Filterwheel:
		ptr = new FilterWheelI(
			DeviceAccessor<astro::camera::FilterWheelPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Focuser:
		ptr = new FocuserI(
			DeviceAccessor<astro::camera::FocuserPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Guiderport:
		ptr = new GuiderPortI(
			DeviceAccessor<astro::camera::GuiderPortPtr>(
				_repository).get(devicename));
		break;

	case astro::DeviceName::Module:
		throw NotImplemented("no module access through devices");

	case astro::DeviceName::Mount:
		throw NotImplemented("mount access not yet implemented");
	};

	devices.insert(std::make_pair(name, ptr));
	return ptr;
}

void	DeviceServantLocator::finished(const Ice::Current& current,
		const Ice::ObjectPtr& servant,
		const Ice::LocalObjectPtr& cookie) {
}

void	DeviceServantLocator::deactivate(const std::string& category) {
}

} // namespace snowstar
