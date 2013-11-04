/*
 * DriverModuleActivator_impl.cpp -- implementation of driver module activator
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#include <DriverModuleActivator_impl.h>
#include <DriverModule_impl.h>
#include <exceptions.hh>

namespace Astro {

/**
 * \brief request that a certain DriverModule be activated
 */
PortableServer::Servant DriverModuleActivator_impl::incarnate(
	const PortableServer::ObjectId& oid,
	PortableServer::POA_ptr poa
) throw (CORBA::SystemException, PortableServer::ForwardRequest) {
	// extract the driver module name from the object id
	std::string	modid;
	try {
		modid = PortableServer::ObjectId_to_string(oid);
	} catch (const CORBA::BAD_PARAM&) {
		throw CORBA::OBJECT_NOT_EXIST();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "activating module %s", modid.c_str());

	// check syntax
	if (modid.size() < 7) {
		throw CORBA::OBJECT_NOT_EXIST();
	}

	// extract the  the module name from the object id
	std::string	modname = modid.substr(7);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module name: %s", modname.c_str());

	// get the ModulePtr from the repository, and ensure it is open
	astro::module::ModulePtr	modptr;
	try {
		modptr = repository.getModule(modname);
		modptr->open();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "module %s problem: %s",
			modname.c_str(), x.what());
		NotFound	notfound;
		notfound.cause = CORBA::string_dup(x.what());
		throw notfound;
	}

	// create the servant
	Astro::DriverModule_impl	*drivermodule
		= new Astro::DriverModule_impl(modptr);

	// return the servant
	return drivermodule;
}

/**
 * \brief Etherelize the servant
 */
void	DriverModuleActivator_impl::etherealize(
	const PortableServer::ObjectId& oid,
	PortableServer::POA_ptr		poa,
	PortableServer::Servant		serv,
	CORBA::Boolean			cleanup_in_progress,
	CORBA::Boolean			remaining_activations
) throw(CORBA::SystemException) {

	// get the object id, for logging
	std::string	modname = PortableServer::ObjectId_to_string(oid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cleanup of driver with oid %s",
		modname.c_str());

	// if no other activations, delete the servant
	if (!remaining_activations) {
		delete serv;
	}
}

} // namespace Astro
