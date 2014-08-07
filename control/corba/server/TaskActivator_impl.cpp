/*
 * TaskActivator_impl.cpp -- Activator to activate Task servcants
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskActivator_impl.h>
#include <Task_impl.h>
#include <sys/stat.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <AstroFilterfunc.h>

namespace Astro {

/**
 * \brief Incarnate a servant for an image
 */
PortableServer::Servant	TaskActivator_impl::incarnate(
				const PortableServer::ObjectId& oid,
				PortableServer::POA_ptr /* poa */
	) throw (CORBA::SystemException, PortableServer::ForwardRequest) {
	// the object id encodes the file name, so first have to convert
	// the object id into a file name
	std::string	idstring;
	try {
		idstring = PortableServer::ObjectId_to_string(oid);
	} catch (const CORBA::BAD_PARAM&) {
		throw CORBA::OBJECT_NOT_EXIST();
	}
	long	id = stoi(idstring);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct servant for task: %ld", id);

	return new Astro::Task_impl(tasktable, id);
}

/**
 * \brief Etherealize a servant
 */
void	TaskActivator_impl::etherealize(
		const PortableServer::ObjectId&	oid,
		PortableServer::POA_ptr		/* poa */,
		PortableServer::Servant		serv,
		CORBA::Boolean			/* cleanup_in_progress */,
		CORBA::Boolean			remaining_activations
	) throw (CORBA::SystemException) {

	// get the file name
	std::string	idstring;
        try {
		idstring = PortableServer::ObjectId_to_string(oid);
        } catch (const CORBA::BAD_PARAM&) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have not filename");
        }

	// when there are no remaining activations, remove the image
	// from the image directory
	if (remaining_activations) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "remaining activations");
		return;
	}
		
	// remove the servant
	delete serv;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "servant deleted");

	// XXX remove the task from the database
}

} // namespace Astro
