/*
 * ServantBuilder.h -- a template to build servants for Ptr objects
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ServantBuilder_h
#define _ServantBuilder_h

#include <Conversions.h>
#include <OrbSingleton.h>
#include <PoaNameMap.h>
#include <AstroLocator.h>

namespace Astro {

/**
 * \brief A class to build device servants from device ptrs
 *
 * \param device	the device type in the astro::camera for which a
 *			servant should be constructed
 * \param device_impl	the corba implementation class type associated with
 *			this device
 */
template<typename device, typename device_impl>
class ServantBuilder {
	astro::device::DeviceLocatorPtr	_locator;
	OrbSingleton	orb;
	PortableServer::POA_var	poa;
public:
	typedef	typename device_impl::device_type	device_type;
	typedef typename device_type::sharedptr	sharedptr;
	typedef typename device::_ptr_type	_ptr_type;
	ServantBuilder(astro::device::DeviceLocatorPtr locator
		= astro::device::DeviceLocatorPtr());
	_ptr_type	operator()(const std::string& name);
	_ptr_type	operator()(sharedptr& devptr);
};

/**
 * \brief Construct a ServantBuilder
 *
 * constructs a 
 */
template<typename device, typename device_impl>
ServantBuilder<device, device_impl>::ServantBuilder(
	astro::device::DeviceLocatorPtr locator) : _locator(locator) {
	PoaName	name = poaname<device>();
	poa = orb.findPOA(name);
}

/**
 * \brief construct a new servant if there is no servant present
 */
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

/**
 * \brief construct a new servant from a shared pointer object
 *
 * This method does not need the locator, as the device is already available
 * as the argument
 */
template<typename device, typename device_impl>
typename device::_ptr_type
ServantBuilder<device, device_impl>::operator()(sharedptr& devptr) {
	std::string	name = devptr->name();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting servant for %s", name.c_str());
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
		poa->activate_object_with_id(oid, new device_impl(devptr));
		CORBA::Object_var	obj = poa->id_to_reference(oid);
		return device::_narrow(obj);
	} catch (...) {
		NotFound	notfound;
                notfound.cause = CORBA::string_dup("device not found");
		throw notfound;
	}
}


} // namespace Astro

#endif /* _ServantBuilder_h */
