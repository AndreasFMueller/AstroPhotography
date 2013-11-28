/*
 * astrod.cpp -- a server that controls astro cameras and accessories
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cstdlib>
#include <includes.h>
#include <AstroDebug.h>
#include <stdexcept>
#include <iostream>
#include <omniORB4/CORBA.h>
#include "Modules_impl.h"
#include "GuiderFactory_impl.h"
#include "Images_impl.h"
#include <NameService.h>
#include <AstroLoader.h>
#include <OrbSingleton.h>
#include <cassert>
#include "DriverModuleActivator_impl.h"
#include "ImageActivator_impl.h"

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


/**
 * \brief Main function for the CORBA server
 */
int	main(int argc, char *argv[]) {
	debugtimeprecision = 3;
	debuglevel = LOG_DEBUG;

	// initialize CORBA
	Astro::OrbSingleton	orb(argc, argv);

	// now parse the command line
	int	c;
	while (EOF != (c = getopt(argc, argv, "db:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'b':
			Astro::ImageDirectory::basedir(optarg);
			break;
		}

	// starting the astro daemon
	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod starting up");

	// get the POA
	CORBA::Object_var	obj
		= orb.orbvar()->resolve_initial_references("RootPOA");
	PortableServer::POA_var	root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initial poa reference");

	// get the naming service
	Astro::Naming::NameService	nameservice(orb);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a name service object");

	// we want a separate child POA for the Modules object, because
	// we want that object reference to be persistent
	POABuilder	pb(root_poa);
	PortableServer::POA_var	modules_poa = pb.build("Modules");

	// create a POA for driver modules
	POABuilderActivator<Astro::DriverModuleActivator_impl>	pb1(modules_poa);
	PortableServer::POA_var	drivermodules_poa = pb1.build("DriverModules",
		new Astro::DriverModuleActivator_impl());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DriverModuleActivator set");

	// create a POA for Camera objects
	POABuilder	pbcamera(drivermodules_poa);
	PortableServer::POA_var	camera_poa
		= pbcamera.build("Cameras");

	// create a POA for Ccd objects
	POABuilder	pbccd(camera_poa);
	PortableServer::POA_var	ccd_poa
		= pbccd.build("Ccds");

	// create a POA for Cooler objects
	POABuilder	pbcooler(ccd_poa);
	PortableServer::POA_var	cooler_poa
		= pbcooler.build("Coolers");

	// create a POA GuiderPort objects
	POABuilder	pbguiderport(camera_poa);
	PortableServer::POA_var	guiderport_poa
		= pbguiderport.build("GuiderPorts");

	// create a POA for FilterWheel objects
	POABuilder	pbfilterwheel(camera_poa);
	PortableServer::POA_var	filterwheel_poa
		= pbfilterwheel.build("FilterWheels");

	// create a POA for Focuser objects
	POABuilder	pbfocuser(drivermodules_poa);
	PortableServer::POA_var	focuser_poa
		= pbfocuser.build("Focusers");
			
	// create the servant and register it with the ORB
	Astro::Modules_impl	*modules = new Astro::Modules_impl();
	PortableServer::ObjectId_var	oid
		= PortableServer::string_to_ObjectId("Modules");
	modules_poa->activate_object_with_id(oid, modules);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules servant created");

	// register the object in the name
	Astro::Naming::Names	names;
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Modules", "object"));

	// register the modules object
	nameservice.bind(names, modules->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules bound");

	// create a servant for the guider factory
	astro::module::Repository	repository;
	astro::guiding::GuiderFactoryPtr
		gfptr(new astro::guiding::GuiderFactory(repository));
	Astro::GuiderFactory_impl	*guiderfactory
		= new Astro::GuiderFactory_impl(gfptr);
	PortableServer::ObjectId_var	guiderfactorysid
		= root_poa->activate_object(guiderfactory);

	// register the GuiderFactory object
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("GuiderFactory", "object"));
	nameservice.bind(names, guiderfactory->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GuiderFactory object bound");

	// create a POA for guiders
	POABuilder	pbguider(root_poa);
	PortableServer::POA_var	guider_poa
		= pbguider.build("Guiders");

	// create a servant for images
	Astro::Images_impl	*images = new Astro::Images_impl();
	PortableServer::ObjectId_var	imagessid
		= root_poa->activate_object(images);

	// register the Images servant
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Images", "object"));
	nameservice.bind(names, images->_this());

	// a POA for images
	POABuilderActivator<Astro::ImageActivator_impl>	pb2(root_poa);
	PortableServer::POA_var	images_poa = pb2.build("Images",
		new Astro::ImageActivator_impl());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ImageActivator set");

	// activate the POA manager
	PortableServer::POAManager_var	pman = root_poa->the_POAManager();
	pman->activate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "poa manager activated");

	// run the orb
	orb.orbvar()->run();
	orb.orbvar()->destroy();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "astrod exiting");
	return EXIT_SUCCESS;
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "astrod terminated by exception: " << x.what()
			<< std::endl;
	}
	exit(EXIT_FAILURE);
}
