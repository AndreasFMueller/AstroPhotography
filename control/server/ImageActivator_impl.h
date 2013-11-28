/*
 * ImageActivator_impl.h -- Activator to activate Image servants
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageActivator_impl_h
#define _ImageActivator_impl_h

#include <omniORB4/CORBA.h>
#include <AstroLoader.h>
#include <ImageDirectory.h>

namespace Astro {

class ImageActivator_impl
	: public virtual POA_PortableServer::ServantActivator,
	  public ImageDirectory {
public:
	ImageActivator_impl() { }

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

#endif /* _ImageActivator_impl_h */
