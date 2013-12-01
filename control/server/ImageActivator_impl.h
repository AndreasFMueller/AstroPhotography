/*
 * ImageActivator_impl.h -- Activator to activate Image servants
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageActivator_impl_h
#define _ImageActivator_impl_h

#include <omniORB4/CORBA.h>
#include <AstroLoader.h>
#include <ImageObjectDirectory.h>

namespace Astro {

/**
 * \brief Activator implementation for images
 *
 * Images in the image directory are not automatically activated as 
 * CORBA objects. But when a client requests an image, a new object
 * reference is created and a servant is activated.
 */
class ImageActivator_impl
	: public virtual POA_PortableServer::ServantActivator,
	  public ImageObjectDirectory {
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
