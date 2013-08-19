/*
 * DeviceLocator_impl.cpp -- Driver module implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "DeviceLocator_impl.h"
#include "GuiderPort_impl.h"
#include "FilterWheel_impl.h"
#include "Camera_impl.h"
#include <AstroDebug.h>

namespace Astro {

char	*DeviceLocator_impl::getName() {
	return ::CORBA::string_dup(_locator->getName().c_str());
}

char	*DeviceLocator_impl::getVersion() {
	return ::CORBA::string_dup(_locator->getVersion().c_str());
}

::Astro::DeviceLocator::DeviceNameList	*DeviceLocator_impl::getDevicelist(::Astro::DeviceLocator::device_type devicetype) {
	astro::device::DeviceLocator::device_type	type
		= astro::device::DeviceLocator::CAMERA;
		// initialize to silence compiler
	switch (devicetype) {
	case ::Astro::DeviceLocator::DEVICE_CAMERA:
		type = astro::device::DeviceLocator::CAMERA;
		break;
	case ::Astro::DeviceLocator::DEVICE_FOCUSER:
		type = astro::device::DeviceLocator::FOCUSER;
		break;
	case ::Astro::DeviceLocator::DEVICE_GUIDERPORT:
		type = astro::device::DeviceLocator::GUIDERPORT;
		break;
	case ::Astro::DeviceLocator::DEVICE_FILTERWHEEL:
		type = astro::device::DeviceLocator::FILTERWHEEL;
		break;
	}
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
 * \brief
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
