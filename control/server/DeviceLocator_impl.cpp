/*
 * DeviceLocator_impl.cpp -- Driver module implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DeviceLocator_impl.h"
#include "GuiderPort_impl.h"
#include "FilterWheel_impl.h"
#include "Camera_impl.h"
#include "Cooler_impl.h"
#include "Ccd_impl.h"
#include "Focuser_impl.h"
#include <AstroDebug.h>
#include <Conversions.h>
#include <OrbSingleton.h>
#include <AstroLocator.h>
#include <PoaNameMap.h>
#include <ServantBuilder.h>

namespace Astro {

//////////////////////////////////////////////////////////////////////
// DeviceLocator_impl implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Get the name of the DeviceLocator
 */
char	*DeviceLocator_impl::getName() {
	return ::CORBA::string_dup(_locator->getName().c_str());
}

/**
 * \brief Get the version of the DeviceLocator
 */
char	*DeviceLocator_impl::getVersion() {
	return ::CORBA::string_dup(_locator->getVersion().c_str());
}

/**
 * \brief Get the list of device names for a given type
 */
::Astro::DeviceLocator::DeviceNameList	*DeviceLocator_impl::getDevicelist(
	::Astro::DeviceLocator::device_type devicetype) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for device type %d",
		devicetype);
	astro::DeviceName::device_type	type
		= astro::convert(devicetype);
	std::vector<std::string>	devices
		= _locator->getDevicelist(type);

	Astro::DeviceLocator::DeviceNameList	*result
		= new Astro::DeviceLocator::DeviceNameList();
	result->length(devices.size());
	std::vector<std::string>::const_iterator	i;
	int	j = 0;
	for (i = devices.begin(); i != devices.end(); i++) {
		(*result)[j++] = ::CORBA::string_dup(i->c_str());
	}
	return result;
}

/**
 * \brief Get A Camera of a given name
 */
Camera_ptr	DeviceLocator_impl::getCamera(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera %s", name);
	std::string	cameraname(name);

	// use the ServantBuilder
	ServantBuilder<Camera, Camera_impl>	servantbuilder(_locator);
	return servantbuilder(cameraname);
}

/**
 * \brief Get A Ccd of a given name
 */
Ccd_ptr	DeviceLocator_impl::getCcd(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ccd %s", name);
	std::string	ccdname(name);

	// use the ServantBuilder
	ServantBuilder<Ccd, Ccd_impl>	servantbuilder(_locator);
	return servantbuilder(ccdname);
}

/**
 * \brief Get a GuiderPort with a given name
 */
GuiderPort_ptr DeviceLocator_impl::getGuiderPort(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get guiderport %s", name);
	std::string	guiderportname(name);

	// use the ServantBuilder
	ServantBuilder<GuiderPort, GuiderPort_impl> servantbuilder(_locator);
	return servantbuilder(guiderportname);
}

/**
 * \brief Get a FilterWheel of a given name
 */
FilterWheel_ptr	DeviceLocator_impl::getFilterWheel(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get filterwheel %s", name);
	std::string	filterwheelname(name);

	// use the ServantBuilder
	ServantBuilder<FilterWheel, FilterWheel_impl> servantbuilder(_locator);
	return servantbuilder(filterwheelname);
}

/**
 * \brief Get a Cooler of a given name
 */
Cooler_ptr	DeviceLocator_impl::getCooler(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get cooler %s", name);
	std::string	coolername(name);

	// use the ServantBuilder
	ServantBuilder<Cooler, Cooler_impl> servantbuilder(_locator);
	return servantbuilder(coolername);
}

/**
 * \brief Get a Focuser of a given name
 */
Focuser_ptr	DeviceLocator_impl::getFocuser(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get focuser %s", name);
	std::string	focusername(name);

	// use the ServantBuilder
	ServantBuilder<Focuser, Focuser_impl> servantbuilder(_locator);
	return servantbuilder(focusername);
}

} // namespace astro
