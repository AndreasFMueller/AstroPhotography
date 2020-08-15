/*
 * Server.cpp -- snowstar server implementation class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "Server.h"
#include <Ice/Ice.h>
#include <Ice/Properties.h>
#include <Ice/Initialize.h>
#include <cstdlib>
#include <iostream>
#include <DevicesI.h>
#include <ImagesI.h>
#include <AstroDebug.h>
#include <DeviceServantLocator.h>
#include <ImageLocator.h>
#include <AstroTask.h>
#include <TaskQueueI.h>
#include <TaskLocator.h>
#include <GuiderFactoryI.h>
#include <GuiderLocator.h>
#include <ModulesI.h>
#include <DriverModuleLocator.h>
#include <DeviceLocatorLocator.h>
#include <FocusingFactoryI.h>
#include <FocusingLocator.h>
#include <AstroConfig.h>
#include <repository.h>
#include <RepositoriesI.h>
#include <RepositoryLocator.h>
#include <AstroDiscovery.h>
#include <AstroFormat.h>
#include <InstrumentLocator.h>
#include <InstrumentsI.h>
#include <CommunicatorSingleton.h>
#include <AstroEvent.h>
#include <EventHandlerI.h>
#include <EventServantLocator.h>
#include <ConfigurationI.h>
#include <DaemonI.h>
#include <GatewayI.h>

namespace snowstar {

// name of the service
static astro::config::ConfigurationKey	_service_name_key(
	"global", "service", "name");
static astro::config::ConfigurationRegister
	_service_name_registration(_service_name_key, "name of the service");

// instruments service
static astro::config::ConfigurationKey	_service_instruments_key(
	"snowstar", "service", "instruments");
static astro::config::ConfigurationRegister	_service_instruments_registration(
	_service_instruments_key,
	"whether or not the instruments service is offered");

// devices service
static astro::config::ConfigurationKey	_service_devices_key(
	"snowstar", "service", "devices");
static astro::config::ConfigurationRegister	_service_devices_registration(
	_service_devices_key,
	"whether or not the devices service is offered");

// tasks service
static astro::config::ConfigurationKey	_service_tasks_key(
	"snowstar", "service", "tasks");
static astro::config::ConfigurationRegister	_service_tasks_registration(
	_service_tasks_key,
	"whether or not the tasks service is offered");

// guiding service
static astro::config::ConfigurationKey	_service_guiding_key(
	"snowstar", "service", "guiding");
static astro::config::ConfigurationRegister	_service_guiding_registration(
	_service_guiding_key,
	"whether or not guiding services are offered");

// focusing service
static astro::config::ConfigurationKey	_service_focusing_key(
	"snowstar", "service", "focusing");
static astro::config::ConfigurationRegister	_service_focusing_registration(
	_service_focusing_key,
	"whether or not focusing services is offered");

// images service
static astro::config::ConfigurationKey	_service_images_key(
	"snowstar", "service", "images");
static astro::config::ConfigurationRegister	_service_images_registration(
	_service_images_key,
	"whether or not images service is offered");

// repository service
static astro::config::ConfigurationKey	_service_repository_key(
	"snowstar", "service", "repository");
static astro::config::ConfigurationRegister	_service_repository_registration(
	_service_repository_key,
	"whether or not repository service is offered");

// gateway service
static astro::config::ConfigurationKey	_service_gateway_key(
	"snowstar", "service", "gateway");
static astro::config::ConfigurationRegister	_service_gateway_registration(
	_service_gateway_key,
	"whether or not gateway service is offered");

static astro::config::ConfigurationKey	_heartbeat_interval_key(
	"snowstar", "daemon", "heartbeat");
static astro::config::ConfigurationRegister	_heartbeat_interval_registration(
	_heartbeat_interval_key,
	"the default heartbeat interval");

/**
 * \brief Get the services to be activated from the configuration
 */
void	Server::get_configured_services(astro::discover::ServicePublisherPtr sp) {
	if (!sp) {
		return;
	}
	auto configuration = astro::config::Configuration::get();
	if (configuration->get(_service_instruments_key, "no") == "yes") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "instruments enabled");
		sp->set(astro::discover::ServiceSubset::INSTRUMENTS);
	}
	if (configuration->get(_service_devices_key, "yes") == "yes") {
		sp->set(astro::discover::ServiceSubset::DEVICES);
	}
	if (configuration->get(_service_tasks_key, "no") == "yes") {
		sp->set(astro::discover::ServiceSubset::TASKS);
	}
	if (configuration->get(_service_guiding_key, "no") == "yes") {
		sp->set(astro::discover::ServiceSubset::GUIDING);
	}
	if (configuration->get(_service_focusing_key, "no") == "yes") {
		sp->set(astro::discover::ServiceSubset::FOCUSING);
	}
	if (configuration->get(_service_images_key, "yes") == "yes") {
		sp->set(astro::discover::ServiceSubset::IMAGES);
	}
	if (configuration->get(_service_repository_key, "no") == "yes") {
		sp->set(astro::discover::ServiceSubset::REPOSITORY);
	}
	if (configuration->get(_service_gateway_key, "no") == "yes") {
		sp->set(astro::discover::ServiceSubset::GATEWAY);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "configured services: %s",
		sp->toString().c_str());
}

// Ice 3.7 has deprecated ic->stringToIdentity but on MacOS X we are stuck
// with 3.5.1, so this macro works around that problem
#if ICE_INT_VERSION > 30501
#define STRING_TO_IDENTITY(name) Ice::stringToIdentity(name)
#else
#define STRING_TO_IDENTITY(name) ic->stringToIdentity(name)
#endif

/**
 * \brief Add devices servant
 */
void	Server::add_devices_servant() {
	Ice::ObjectPtr	object = new DevicesI(devices);
	adapter->add(object, STRING_TO_IDENTITY("Devices"));
	DeviceServantLocator	*deviceservantlocator
		= new DeviceServantLocator(repository);
	adapter->addServantLocator(deviceservantlocator, "");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices servant added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::DEVICE,
		"Device server ready");

	// add a servant for the modules
	object = new ModulesI();
	adapter->add(object, STRING_TO_IDENTITY("Modules"));
	DriverModuleLocator	*drivermodulelocator
		= new DriverModuleLocator(repository);
	adapter->addServantLocator(drivermodulelocator, "drivermodule");

	// add servant locator for device locator
	DeviceLocatorLocator	*devicelocatorlocator
		= new DeviceLocatorLocator(repository);
	adapter->addServantLocator(devicelocatorlocator, "devicelocator");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Modules servant added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::MODULE,
		"Module server ready");
}

void	Server::add_event_servant() {
	Ice::ObjectPtr	object = new EventHandlerI();
	adapter->add(object, STRING_TO_IDENTITY("Events"));
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::DEBUG,
		"Event server added");
}

void	Server::add_gateway_servant() {
	Ice::ObjectPtr	object = new GatewayI();
	adapter->add(object, STRING_TO_IDENTITY("Gateway"));
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::DEBUG,
		"Gateway server added");
}

void	Server::add_configuration_servant() {
	astro::config::ConfigurationPtr	configuration
		= astro::config::Configuration::get();
	Ice::ObjectPtr	object = new ConfigurationI(configuration);
	adapter->add(object, STRING_TO_IDENTITY("Configuration"));
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::DEBUG,
		"Configuration server added");
}

void	Server::add_daemon_servant() {
	// create the daemon
	DaemonI	*daemon = new DaemonI(*this);
	Ice::ObjectPtr	object = daemon;

	// get the interval from the configuration
	astro::config::ConfigurationPtr	configuration
		= astro::config::Configuration::get();
	std::string	intervalstring = configuration->get(
				_heartbeat_interval_key, "5");
	int	interval = 0;
	try {
		interval = std::stoi(intervalstring);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "invalid interval string '%s': %s",
			intervalstring.c_str(), x.what());
	}
	daemon->interval(interval);

	// add the daemon to the adapter
	adapter->add(object, STRING_TO_IDENTITY("Daemon"));
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::DEBUG,
		"Daemon server added");
}

void	Server::add_images_servant() {
	Ice::ObjectPtr	object = new ImagesI();
	adapter->add(object, STRING_TO_IDENTITY("Images"));
	ImageLocator	*imagelocator = new ImageLocator();
	adapter->addServantLocator(imagelocator, "image");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "images servant locator added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::IMAGE,
		"Image server ready");
}

void	Server::add_tasks_servant() {
	Ice::ObjectPtr	object = new TaskQueueI(taskqueue);
	adapter->add(object, STRING_TO_IDENTITY("Tasks"));
	TaskLocator	*tasklocator = new TaskLocator(database);
	adapter->addServantLocator(tasklocator, "task");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task locator added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::TASK,
		"Task server ready");
}

void	Server::add_instruments_servant() {
	Ice::ObjectPtr	object = new InstrumentsI();
	adapter->add(object, STRING_TO_IDENTITY("Instruments"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Instruments servant added");
	InstrumentLocator	*instrumentlocator = new InstrumentLocator();
	adapter->addServantLocator(instrumentlocator, "instrument");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Instrument servant added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::INSTRUMENT,
		"Instrument server ready");
}

void	Server::add_repository_servant() {
	_repositories = new RepositoriesI();
	Ice::ObjectPtr	object = _repositories;
	adapter->add(object, STRING_TO_IDENTITY("Repositories"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Repositories servant added");
	RepositoryLocator	*repolocator = new RepositoryLocator();
	adapter->addServantLocator(repolocator, "repository");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Repository servant added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::REPOSITORY,
		"Repository server ready");
}

void	Server::add_guiding_servant() {
	GuiderLocator	*guiderlocator = new GuiderLocator();
	Ice::ObjectPtr	object = new GuiderFactoryI(guiderlocator);
	adapter->add(object, STRING_TO_IDENTITY("Guiders"));
	adapter->addServantLocator(guiderlocator, "guider");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::GUIDE,
		"Guider server ready");
}

void	Server::add_focusing_servant() {
	Ice::ObjectPtr	object = new FocusingFactoryI();
	adapter->add(object, STRING_TO_IDENTITY("FocusingFactory"));
	FocusingLocator	*focusinglocator = new FocusingLocator();
	adapter->addServantLocator(focusinglocator, "focusing");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Focusing servant added");
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::FOCUS,
		"Focusing server ready");
}

static astro::persistence::Database	getdatabase(const std::string& databasefilename) {
	astro::persistence::DatabaseFactory     dbfactory;
	return dbfactory.get(databasefilename);
}

Server::Server(Ice::CommunicatorPtr _ic, const std::string& dbfilename)
	: ic(_ic),
	  repository(astro::module::ModuleRepository::get()),
	  devices(repository),
	  database(getdatabase(dbfilename)),
	  taskqueue(database) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating a server");

	// get the times
	time(&_start_time);
	struct tms	t;
	_start_clock = times(&t);

	// activate the event log
	astro::events::EventHandler::active(true);
	astro::event(EVENT_GLOBAL, astro::events::INFO,
		astro::events::Event::SERVER,
		"snowstar server startup");

	// determine which service name to use
	astro::discover::ServiceLocation&	location
		= astro::discover::ServiceLocation::get();
	sp = astro::discover::ServicePublisher::get(location.servicename(),
			location.port());
	if (location.ssl()) {
		sps = astro::discover::ServicePublisher::get(
			location.servicename() + "-ssl", location.sslport());
	}

	// find out which services are configured
	get_configured_services(sp);
	get_configured_services(sps);
	// publish the service name
	sp->publish();
	if (sps) { sps->publish(); }
	debug(LOG_DEBUG, DEBUG_LOG, 0, "services pubilshed");

	// initialize the guider factory
	astro::guiding::GuiderFactory::initialize(repository, database);

	// initialize servants

	// create the adapter
	std::string	connectstring = astro::stringprintf(
		"default -p %hu", location.port());
	if (location.sslport() > 0) {
		connectstring += astro::stringprintf(" -p %hu:ssl",
			location.sslport());
	}
	adapter = ic->createObjectAdapterWithEndpoints("Astro",
			connectstring);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adapters created");

	// add a servant for events to the adapter
	add_event_servant();

	// add a servant for configuration data
	add_configuration_servant();
	add_daemon_servant();

	// add a servant for devices to the device adapter
	if (sp->has(astro::discover::ServiceSubset::DEVICES)) {
		add_devices_servant();
	}

	// add a servant for images to the adapter
	if (sp->has(astro::discover::ServiceSubset::IMAGES) ||
		sp->has(astro::discover::ServiceSubset::GUIDING) ||
		sp->has(astro::discover::ServiceSubset::FOCUSING)) {
		add_images_servant();
	}

	// add a servant for taskqueue to the adapter
	if (sp->has(astro::discover::ServiceSubset::TASKS)) {
		add_tasks_servant();
	}

	// add a servant for the guider factory
	if (sp->has(astro::discover::ServiceSubset::GUIDING)) {
		add_guiding_servant();
	}

	// add a servant for Focusing
	if (sp->has(astro::discover::ServiceSubset::FOCUSING)) {
		add_focusing_servant();
	}

	// add a servant for Repositories
	if (sp->has(astro::discover::ServiceSubset::REPOSITORY)) {
		add_repository_servant();
	}

	// add a servant for Instruments
	if (sp->has(astro::discover::ServiceSubset::INSTRUMENTS)) {
		add_instruments_servant();
	}

	// add a servant for Gateway
	if (sp->has(astro::discover::ServiceSubset::GATEWAY)) {
		add_gateway_servant();
	}

	// activate the adapter
	adapter->activate();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adapter activated");
}

void	Server::waitForShutdown() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for shutdown");
	ic->waitForShutdown();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown complete");
}

void	Server::reloadRepositories() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reload repositories called");
	if (!_repositories) {
		return;
	}
	_repositories->reloadDB();
}

} // namespace snowstar
