/*
 * POABuilder.cpp -- a server that controls astro cameras and accessories
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <POABuilder.h>
#include <cstdlib>
#include <includes.h>
#include <AstroDebug.h>
#include <omniORB4/CORBA.h>
#include <cassert>

namespace astro {

PortableServer::POA_var	POABuilder::build(const std::string& poaname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "build POA named '%s'", poaname.c_str());
	// build a policy list for the POA
	CORBA::PolicyList	policy_list;
	PortableServer::IdAssignmentPolicy_var	assign
		= _poa->create_id_assignment_policy(PortableServer::USER_ID);
	PortableServer::LifespanPolicy_var	lifespan
		= _poa->create_lifespan_policy(PortableServer::TRANSIENT);
	policy_list.length(2);
	policy_list[0] = PortableServer::IdAssignmentPolicy::_duplicate(assign);
	policy_list[1] = PortableServer::LifespanPolicy::_duplicate(lifespan);

	// now create the POA
	PortableServer::POA_var result = _poa->create_POA(poaname.c_str(),
		_poa->the_POAManager(), policy_list);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "POA '%s' constructed", poaname.c_str());

	// cleanup
	assign->destroy();
	lifespan->destroy();
	return result;
}

} // namespace astro
