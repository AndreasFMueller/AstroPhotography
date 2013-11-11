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

namespace Astro {

//////////////////////////////////////////////////////////////////////
// template class for building servants
//////////////////////////////////////////////////////////////////////

template<typename device, typename device_impl>
class ServantBuilder {
	astro::device::DeviceLocatorPtr	_locator;
	OrbSingleton	orb;
	PortableServer::POA_var	poa;
public:
	typedef	typename device_impl::device_type	device_type;
	typedef typename device_type::sharedptr	sharedptr;
	typedef typename device::_ptr_type	_ptr_type;
	ServantBuilder(astro::device::DeviceLocatorPtr locator);
	_ptr_type	operator()(const std::string& name);
};

template<typename device, typename device_impl>
ServantBuilder<device, device_impl>::ServantBuilder(
	astro::device::DeviceLocatorPtr locator) : _locator(locator) {
	PoaName	name = poaname<device>();
	poa = orb.findPOA(name);
}

template<typename device, typename device_impl>
typename device::_ptr_type	ServantBuilder<device, device_impl>::operator()(
		const std::string& name) {
	// get the object id
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId(name.c_str());

	// find out whether the servant already exists
	try {
		CORBA::Object_var	obj = poa->id_to_reference(oid);
		return device::_narrow(obj);
	} catch (PortableServer::POA::ObjectNotActive&) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s has no servant yet",
			name.c_str());
	}

	// create a new servant
	try {
		astro::device::LocatorAdapter<device_type>	la(_locator);
		sharedptr	dptr = la.get(name);
		poa->activate_object_with_id(oid, new device_impl(dptr));

		CORBA::Object_var	obj = poa->id_to_reference(oid);
		return device::_narrow(obj);
	} catch (...) {
		NotFound	notfound;
                notfound.cause = CORBA::string_dup("device not found");
		throw notfound;
	}
}

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
