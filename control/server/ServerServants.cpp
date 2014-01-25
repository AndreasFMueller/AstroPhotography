/*
 * ServerServants.cpp
 *
 * () 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ServerServants.h>
#include <omniORB4/CORBA.h>
#include <cassert>
#include <NameService.h>
#include <POABuilder.h>
#include "DriverModuleActivator_impl.h"
#include "ImageActivator_impl.h"
#include "TaskActivator_impl.h"

#include "Modules_impl.h"
#include "TaskQueue_impl.h"
#include "Images_impl.h"
#include "GuiderFactory_impl.h"

namespace astro {

CORBA::Object_var	obj;
PortableServer::ObjectId_var	oid;

ServerServants::ServerServants(Astro::OrbSingleton& orb,
	persistence::Database database)
		: _database(database),
		  taskqueuebackend(database) {

	// get the POA
//	CORBA::Object_ptr	obj
obj		= orb.orbvar()->resolve_initial_references("RootPOA");
CORBA::Object::_duplicate(obj);
	root_poa = PortableServer::POA::_narrow(obj);
	assert(!CORBA::is_nil(root_poa));
	PortableServer::POA::_duplicate(root_poa);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initial poa reference");

	// get the naming service
	Astro::Naming::NameService	nameservice(orb);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a name service object");

	// we want a separate child POA for the Modules object, because
	// we want that object reference to be persistent
	POABuilder	pb(root_poa);
	modules_poa = pb.build("Modules");

	// create a POA for driver modules
	POABuilderActivator<Astro::DriverModuleActivator_impl>	pb1(modules_poa);
	drivermodules_poa = pb1.build("DriverModules",
		new Astro::DriverModuleActivator_impl());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "DriverModuleActivator set");

	// create a POA for Camera objects
	POABuilder	pbcamera(drivermodules_poa);
	camera_poa = pbcamera.build("Cameras");
	PortableServer::POA::_duplicate(camera_poa);

	// create a POA for Ccd objects
	POABuilder	pbccd(camera_poa);
	ccd_poa = pbccd.build("Ccds");

	// create a POA for Cooler objects
	POABuilder	pbcooler(ccd_poa);
	cooler_poa = pbcooler.build("Coolers");

	// create a POA GuiderPort objects
	POABuilder	pbguiderport(camera_poa);
	guiderport_poa = pbguiderport.build("GuiderPorts");

	// create a POA for FilterWheel objects
	POABuilder	pbfilterwheel(camera_poa);
	filterwheel_poa = pbfilterwheel.build("FilterWheels");

	// create a POA for Focuser objects
	POABuilder	pbfocuser(drivermodules_poa);
	focuser_poa = pbfocuser.build("Focusers");
			
	// create the servant and register it with the ORB
	_modules = new Astro::Modules_impl();
//	PortableServer::ObjectId_var	oid
oid		= PortableServer::string_to_ObjectId("Modules");
	modules_poa->activate_object_with_id(oid, _modules);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules servant created");

	// register the object in the name
	Astro::Naming::Names	names;
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Modules", "object"));

	// register the modules object
	nameservice.bind(names, _modules->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules bound");

	// create a servant for the guider factory
	astro::guiding::GuiderFactoryPtr
		gfptr(new astro::guiding::GuiderFactory(repository));
	_guiderfactory = new Astro::GuiderFactory_impl(gfptr);
	guiderfactorysid = root_poa->activate_object(_guiderfactory);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderfactory %p", _guiderfactory);

	// register the GuiderFactory object
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("GuiderFactory", "object"));
	nameservice.bind(names, _guiderfactory->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "GuiderFactory object bound");

	// create a POA for guiders
	POABuilder	pbguider(root_poa);
	guider_poa = pbguider.build("Guiders");

	// create a servant for images
	_images = new Astro::Images_impl();
	imagessid = root_poa->activate_object(_images);

	// register the Images servant
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("Images", "object"));
	nameservice.bind(names, _images->_this());

	// a POA for images
	POABuilderActivator<Astro::ImageActivator_impl>	pb2(root_poa);
	images_poa = pb2.build("Images", new Astro::ImageActivator_impl());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ImageActivator set");

	// create the servant for TaskQueue
	_taskqueue = new Astro::TaskQueue_impl(taskqueuebackend);
	taskqueuesid = root_poa->activate_object(_taskqueue);

	// register the TaskQueue servant
	names.clear();
	names.push_back(Astro::Naming::Name("Astro", "context"));
	names.push_back(Astro::Naming::Name("TaskQueue", "object"));
	nameservice.bind(names, _taskqueue->_this());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task queue servant activated");

	// a POA for Tasks
	POABuilderActivator<Astro::TaskActivator_impl>	pb3(root_poa);
	tasks_poa = pb3.build("Tasks", new Astro::TaskActivator_impl(database));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "TaskActivator set");

	// activate the POA manager
	root_poa->the_POAManager()->activate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "poa manager activated");
}

ServerServants::~ServerServants() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Servants destroyed");
}

static Servants	_servants = NULL;

Servants ServantsFactory::get(persistence::Database _database) {
	if (_servants) {
		return _servants;
	}
	Astro::OrbSingleton	orb;
	_servants = Servants(new ServerServants(orb, _database));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "servants initialized: %p", &*_servants);
	return _servants;
}

Servants ServantsFactory::get() {
	if (_servants) {
		return _servants;
	}
	throw std::runtime_error("servants not initialized");
}


} // namespace astro

