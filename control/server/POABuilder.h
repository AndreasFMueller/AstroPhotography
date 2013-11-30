/*
 * POABuilder.h -- auxiliary class to build POAs
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _POABuilder_h
#define _POABuilder_h

#include <cstdlib>
#include <includes.h>
#include <AstroDebug.h>
#include <omniORB4/CORBA.h>
#include <OrbSingleton.h>

namespace astro {

//////////////////////////////////////////////////////////////////////
// POABuilder class: builds standard POAs
//////////////////////////////////////////////////////////////////////

/**
 * \brief Auxiliary to build a POA
 */
class POABuilder {
	PortableServer::POA_var	_poa;
public:
	POABuilder(PortableServer::POA_ptr poaptr) : _poa(poaptr) { }
	PortableServer::POA_var	build(const std::string& poaname);
};

//////////////////////////////////////////////////////////////////////
// POABuilderActivator template: builds POAs that use an activator
//////////////////////////////////////////////////////////////////////
template <typename activator>
class POABuilderActivator {
	PortableServer::POA_var	_poa;
public:
	POABuilderActivator(PortableServer::POA_ptr poaptr) : _poa(poaptr) { }
	PortableServer::POA_var	build(const std::string& poaname,
		activator *theactivator);
};

template<typename activator>
PortableServer::POA_var	POABuilderActivator<activator>::build(
		const std::string& poaname, activator *theactivator) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "build a POA named '%s' with activator",
		poaname.c_str());
	// create the policy
	PortableServer::IdAssignmentPolicy_var	assign
		= _poa->create_id_assignment_policy(PortableServer::USER_ID);
	PortableServer::RequestProcessingPolicy_var	requestprocessing
		= _poa->create_request_processing_policy(
			PortableServer::USE_SERVANT_MANAGER);
	CORBA::PolicyList	policy_list;
	policy_list.length(2);
	policy_list[0] = PortableServer::IdAssignmentPolicy::_duplicate(assign);
	policy_list[1] = PortableServer::RequestProcessingPolicy::_duplicate(
				requestprocessing);

	// now build the POA
	PortableServer::POA_var	result_poa
		= _poa->create_POA(poaname.c_str(),
			_poa->the_POAManager(), policy_list);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created POA '%s' with activator");

	// cleanup
	assign->destroy();
	requestprocessing->destroy();

	// now assign the activator
	PortableServer::ServantManager_var	activator_ref
		= theactivator->_this();
	result_poa->set_servant_manager(activator_ref);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Activator set");

	// return the constructed POA
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"POA construction with activator complete");
	return result_poa;
}

} // namespace astro

#endif /* _POABuilder_h */
