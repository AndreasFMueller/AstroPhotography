/*
 * DriverModuleActivator_impl.h -- Activator to activate driver modules
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DriverModuleActivator_impl_h
#define _DriverModuleActivator_impl_h

#include <omniORB4/CORBA.h>
#include <AstroLoader.h>

namespace Astro {

class DriverModuleActivator_impl
	: public virtual POA_PortableServer::ServantActivator {
	astro::module::Repository       repository;
public:
	DriverModuleActivator_impl();

	virtual PortableServer::Servant incarnate(
		const PortableServer::ObjectId& oid,
		PortableServer::POA_ptr poa
	) throw (
		CORBA::SystemException, PortableServer::ForwardRequest
	);

	virtual void	etherealize(
		const PortableServer::ObjectId& oid,
		PortableServer::POA_ptr		poa,
		PortableServer::Servant		serv,
		CORBA::Boolean			cleanup_in_progress,
		CORBA::Boolean			remaining_activations
	) throw(CORBA::SystemException);
		
};

} // namespace Astro

#endif /* _DriverModuleActivator_impl_h */
