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
#include "Focuser_impl.h"
#include <AstroDebug.h>
#include <Conversions.h>

namespace Astro {

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
	astro::device::DeviceLocator::device_type	type
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
 * \brief Get a GuiderPort with a given name
 */
GuiderPort_ptr	DeviceLocator_impl::getGuiderPort(const char *name) {
	// find out whether this guider port has already been retrieved
	std::string	guidername(name);
	astro::camera::GuiderPortPtr	guiderport;
	if (guiderportmap.find(guidername) == guiderportmap.end()) {
		guiderport = _locator->getGuiderPort(guidername);
		guiderportmap.insert(std::make_pair(guidername, guiderport));
	} else {
		guiderport = guiderportmap.find(guidername)->second;
	}
	GuiderPort_impl	*gp = new GuiderPort_impl(guiderport);
	return gp->_this();
}

/**
 * \brief Get a FilterWheel of a given name
 */
FilterWheel_ptr	DeviceLocator_impl::getFilterWheel(const char *name) {
	// find out whether this guider port has already been retrieved
	std::string	filtername(name);
	astro::camera::FilterWheelPtr	filterwheel;
	if (filterwheelmap.find(filtername) == filterwheelmap.end()) {
		filterwheel = _locator->getFilterWheel(filtername);
		filterwheelmap.insert(std::make_pair(filtername, filterwheel));
	} else {
		filterwheel = filterwheelmap.find(filtername)->second;
	}
	FilterWheel_impl	*gp = new FilterWheel_impl(filterwheel);
	return gp->_this();
}

/**
 * \brief Get a Cooler of a given name
 */
Cooler_ptr	DeviceLocator_impl::getCooler(const char *name) {
	// find out whether this guider port has already been retrieved
	std::string	coolername(name);
	astro::camera::CoolerPtr	cooler;
	if (coolermap.find(coolername) == coolermap.end()) {
		cooler = _locator->getCooler(coolername);
		coolermap.insert(std::make_pair(coolername, cooler));
	} else {
		cooler = coolermap.find(coolername)->second;
	}
	Cooler_impl	*gp = new Cooler_impl(cooler);
	return gp->_this();
}

/**
 * \brief Get a Focuser of a given name
 */
Focuser_ptr	DeviceLocator_impl::getFocuser(const char *name) {
	// find out whether this guider port has already been retrieved
	std::string	focusername(name);
	astro::camera::FocuserPtr	focuser;
	if (focusermap.find(focusername) == focusermap.end()) {
		focuser = _locator->getFocuser(focusername);
		focusermap.insert(std::make_pair(focusername, focuser));
	} else {
		focuser = focusermap.find(focusername)->second;
	}
	Focuser_impl	*gp = new Focuser_impl(focuser);
	return gp->_this();
}

/**
 * \brief Get A Camera of a given name
 */
Camera_ptr	DeviceLocator_impl::getCamera(const char *name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving camera %s", name);
	// find out whether this guider port has already been retrieved
	std::string	cameraname(name);
	astro::camera::CameraPtr	camera;
	if (cameramap.find(cameraname) == cameramap.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new camera: %s", name);
		camera = _locator->getCamera(cameraname);
		cameramap.insert(std::make_pair(cameraname, camera));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera %s already open", name);
		camera = cameramap.find(cameraname)->second;
	}
	Camera_impl	*c = new Camera_impl(camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created camera at %p", c);
	return c->_this();
}

} // namespace astro
