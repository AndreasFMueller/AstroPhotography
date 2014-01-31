/*
 * ServerServants.h -- a singleton class that makes available all servants
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ServerServants_h
#define _ServerServants_h

#include <OrbSingleton.h>
#include <AstroPersistence.h>
#include <AstroTask.h>
#include <AstroLoader.h>

namespace Astro {

class Modules_impl;
class Images_impl;
class GuiderFactory_impl;
class TaskQueue_impl;

} // namespace Astro

namespace astro {

class ServerServants {
private:
	Astro::Modules_impl		*_modules;
	Astro::Images_impl		*_images;
	Astro::GuiderFactory_impl	*_guiderfactory;
	Astro::TaskQueue_impl		*_taskqueue;
public:
	Astro::Modules_impl		*modules() { return _modules; }
	Astro::Images_impl		*images() { return _images; }
	Astro::GuiderFactory_impl	*guiderfactory() {
						return _guiderfactory;
					}
	Astro::TaskQueue_impl		*taskqueue() { return _taskqueue; }
private:
	PortableServer::POA_var	root_poa;
	PortableServer::POA_var	modules_poa;
	PortableServer::POA_var	drivermodules_poa;
	PortableServer::POA_var	camera_poa;
	PortableServer::POA_var	ccd_poa;
	PortableServer::POA_var	cooler_poa;
	PortableServer::POA_var	guiderport_poa;
	PortableServer::POA_var	filterwheel_poa;
	PortableServer::POA_var	focuser_poa;
	PortableServer::POA_var	guider_poa;
	PortableServer::POA_var	images_poa;
	PortableServer::POA_var	tasks_poa;
	PortableServer::POA_var	_poa;
	PortableServer::ObjectId_var	guiderfactorysid;
	PortableServer::ObjectId_var	imagessid;
	PortableServer::ObjectId_var	taskqueuesid;
private:	
	persistence::Database	_database;
public:
	persistence::Database	database() { return _database; }
private:
	astro::task::TaskQueue  	taskqueuebackend;
        astro::module::Repository       repository;
public:
	ServerServants(Astro::OrbSingleton& orb,
		persistence::Database _database);
	~ServerServants();
};




typedef std::shared_ptr<ServerServants>	Servants;

class ServantsFactory {
public:
static Servants	get(persistence::Database _database);
static Servants	get();
};

} // namespace astro

#endif /* _ServerServants_h */
//	PortableServer::ObjectId_var	oid
