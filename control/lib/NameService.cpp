/*
 * NameService.cpp -- function to perform the binding 
 *
 * (c) 2013 Prof Dr Andreas Mueller
 */
#include "NameService.h"
#include <cstdlib>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <string>

using namespace astro;

namespace Astro {
namespace Naming {

/**
 * \brief 
 */
std::string	Name::toString() const {
	return id() + "/" + kind();
}

/**
 * \brief Text representation of a name path
 */
std::string	Names::toString() const {
	std::ostringstream	out;
	const_iterator	i;
	bool	initial = true;
	for (i = begin(); i != end(); i++) {
		if (!initial) {
			out << "/";
		}
		out << i->toString();
		initial = false;
	}
	return out.str();
}

/**
 * \brief Construct a NameService
 *
 * \param orb	The ORB to use to talk to the naming service
 */
NameService::NameService(CORBA::ORB_var orb) {
	try {
		CORBA::Object_var	obj;
		obj = orb->resolve_initial_references("NameService");
		rootContext = CosNaming::NamingContext::_narrow(obj);
		if (CORBA::is_nil(rootContext)) {
			std::string	msg("failed to narrow root naming context");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	} catch (CORBA::NO_RESOURCES&) {
		debug(LOG_ERR, DEBUG_LOG, 0, "omniORB is not configured");
		throw std::runtime_error("omniORB not correctly configured");
	} catch (CORBA::ORB::InvalidName&) {
		debug(LOG_ERR, DEBUG_LOG, 0, "Service required is invalid");
		throw std::runtime_error("service required is invalid");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "naming service initialized");
}

/**
 * \brief Perform a lookup in the naming service
 */
CORBA::Object_var	NameService::lookup(const Names& names) {
	// prepare the path for object lookup in the root context
	CosNaming::Name	name;
	name.length(names.size());
	int	j;
	Names::const_iterator	i;
	for (i = names.begin(), j = 0; i != names.end(); i++, j++) {
		name[j].id = i->id().c_str();
		name[j].kind = i->kind().c_str();
	}
	try {
		return rootContext->resolve(name);
	} catch (CosNaming::NamingContext::NotFound& ex) {
		std::string	msg = stringprintf("%s not found",
			names.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "context not found");
		throw std::runtime_error(msg);
	} catch (CORBA::TRANSIENT& ex) {
		std::string	msg = stringprintf("CORBA TRANSIENT error");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	} catch (CORBA::SystemException& ex) {
		std::string	msg = stringprintf("CORBA System error");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

}


/**
 * \brief Bind an Object to a name in the naming service
 */
void	NameService::bind(const Names& names, CORBA::Object_var obj) {
	// build the context name
	CosNaming::Name	contextName;
	contextName.length(names.size() - 1);
	Names::const_iterator	i;
	int	j;
	for (i = names.begin(), j = 0; j < (int)names.size() - 1; i++, j++) {
		contextName[j].id = i->id().c_str();
		contextName[j].kind = i->kind().c_str();
	}

	// get the object name
	CosNaming::Name	objectName;
	objectName.length(1);
	objectName[0].id = names[names.size() - 1].id().c_str();
	objectName[0].kind = names[names.size() - 1].kind().c_str();

	// create or resolve a context and bind the object
	try {
		// bind the context
		CosNaming::NamingContext_var	context;
		try {
			context = rootContext->bind_new_context(contextName);
		} catch (CosNaming::NamingContext::AlreadyBound& ex) {
			CORBA::Object_var	obj
				= rootContext->resolve(contextName);
			context = CosNaming::NamingContext::_narrow(obj);
			if (CORBA::is_nil(context)) {
				std::cerr << "failed to narrow naming context"
					<< std::endl;
			}
		}

		// bind or rebind the name
		try {
			context->bind(objectName, obj);
		} catch (CosNaming::NamingContext::AlreadyBound& ex) {
			context->rebind(objectName, obj);
		}
	} catch (CORBA::TRANSIENT& ex) {
		throw std::runtime_error("Corba transient exception");
	} catch (CORBA::SystemException& ex) {
		throw std::runtime_error("Corba system exception");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "object now bound");
} 

} // namespace Naming
} // namespace Astro
