/*
 * NiceLocator.cpp -- Locator for nice services
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceLocator.h>
#include <AstroLoader.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <device.h>
#include <NiceDevice.h>
#include <NiceCamera.h>
#include <NiceCcd.h>
#include <NiceGuiderPort.h>
#include <NiceFocuser.h>
#include <NiceAdaptiveOptics.h>
#include <NiceFilterWheel.h>
#include <NiceCooler.h>

namespace astro {
namespace camera {
namespace nice {

using namespace astro::discover;

//////////////////////////////////////////////////////////////////////
// NiceDescriptor module descriptor class
//////////////////////////////////////////////////////////////////////

static std::string	nice_name("nice");
static std::string	nice_version(VERSION);

class NiceDescriptor : public astro::module::ModuleDescriptor {
public:
	NiceDescriptor() { }
	~NiceDescriptor() { }
	virtual std::string	name() const {
		return nice_name;
	}
	virtual std::string	version() const {
		return nice_version;
	}
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};


} // namespace nice
} // namespace camera
} // namespace astro

extern "C"
astro::module::ModuleDescriptor *getDescriptor() {
	return new astro::camera::nice::NiceDescriptor();
}

namespace astro {
namespace camera {
namespace nice {

//////////////////////////////////////////////////////////////////////
// NiceLocator class
//////////////////////////////////////////////////////////////////////
NiceLocator::NiceLocator() {
	discovery = ServiceDiscovery::get();
	discovery->start();
}

NiceLocator::~NiceLocator() {
}

std::string	NiceLocator::getName() const {
	return nice_name;
}

std::string	NiceLocator::getVersion() const {
	return nice_version;
}

/**
 * \brief Get a handle to the modules on a server
 *
 * The NiceLocator class keeps a cache of modules proxies for each service
 * it has talked to.
 */
snowstar::ModulesPrx	NiceLocator::getModules(const std::string& servicename) {
	std::unique_lock<std::mutex>	lock(modules_mtx);
	ModulesMap::iterator	i = modules.find(servicename);
	if (i != modules.end()) {
		return i->second;
	}

	// get a proxy to Modules
	ServiceKey	key = discovery->find(servicename);
	return getModules(key);
}

/**
 * \brief Get a modules proxy from a key
 */
snowstar::ModulesPrx	NiceLocator::getModules(const ServiceKey& key) {
	ServiceObject	object = discovery->find(key);

	// we need a connection 
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(object.connect("Modules"));
	snowstar::ModulesPrx	mprx = snowstar::ModulesPrx::checkedCast(base);

	// store the new proxy in the modules map
	modules.insert(std::make_pair(key.name(), mprx));

	// here we have the new proxy
	return mprx;
}

/**
 * \brief Get a DriverModule proxy for a given module name
 */
snowstar::DriverModulePrx	NiceLocator::getDriverModule(
	const std::string& servicename, const std::string& modulename) {
	return getModules(servicename)->getModule(modulename);
}

/** 
 * \brief Get a driver module to a service key and a module name
 */
snowstar::DriverModulePrx	NiceLocator::getDriverModule(
	const ServiceKey& key, const std::string& modulename) {
	return getModules(key)->getModule(modulename);
}

/**
 * \brief Get a DeviceLocator proxy for a given module name
 */
snowstar::DeviceLocatorPrx	NiceLocator::getLocator(
	const std::string& servicename, const std::string& modulename) {
	snowstar::DriverModulePrx	drivermodule
		= getDriverModule(servicename, modulename);
	if (!drivermodule->hasLocator()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "does not have a a locator");
		throw std::runtime_error("no locator");
	}
	return drivermodule->getDeviceLocator();
}

/**
 * \brief Get the DeviceLocator proxy for a key and modulename
 */
snowstar::DeviceLocatorPrx	NiceLocator::getLocator(const ServiceKey& key,
					const std::string& modulename) {
	snowstar::DriverModulePrx	drivermodule
		= getDriverModule(key, modulename);
	if (!drivermodule->hasLocator()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "does not have a a locator");
		throw std::runtime_error("no locator");
	}
	return drivermodule->getDeviceLocator();
}

/**
 * \brief Get a Device list for a given module
 */
std::vector<std::string>	NiceLocator::getDevicelist(
					DeviceName::device_type device,
					snowstar::DriverModulePrx module) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting devices from module %s",
		module->getName().c_str());
	std::vector<std::string>	result;
	if (!module->hasLocator()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "does not have a a locator");
		return result;
	}
	snowstar::DeviceLocatorPrx locator = module->getDeviceLocator();
	snowstar::DeviceNameList	names
		= locator->getDevicelist(snowstar::convert(device));
	std::copy(names.begin(), names.end(),
		std::back_inserter<std::vector<std::string> >(result));
	return result;
}

/**
 * \brief 
 */
std::vector<std::string>	NiceLocator::getDevicelist(
					DeviceName::device_type device,
					const ServiceKey& key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting list from %s",
		key.toString().c_str());
	std::vector<std::string>	result;

	// we need a connection 
	snowstar::ModulesPrx	modules = getModules(key);

	// remember the Modules
	snowstar::ModuleNameList	list = modules->getModuleNames();
	snowstar::ModuleNameList::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		std::string	name = *i;
		snowstar::DriverModulePrx	module = modules->getModule(*i);
		std::vector<std::string>	names
			= getDevicelist(device, module);
		names = DeviceNicer(key.name())(names);
		std::copy(names.begin(), names.end(),
			std::back_inserter<std::vector<std::string> >(result));
	}

	return result;
}

/**
 * \brief Get a list of all devices available through nice
 *
 * This means iterating through all the available services
 */
std::vector<std::string>	NiceLocator::getDevicelist(
					DeviceName::device_type device) {
	std::vector<std::string>	result;

	// getting a list of available severs
	ServiceDiscovery::ServiceKeySet	services = discovery->list();
	ServiceDiscovery::ServiceKeySet::const_iterator	i;
	for (i = services.begin(); i != services.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "service %s",
			i->toString().c_str());
	}

	// got through the servers and 
	for (i = services.begin(); i != services.end(); i++) {
		std::vector<std::string> names = getDevicelist(device, *i);
		std::copy(names.begin(), names.end(),
			std::back_inserter<std::vector<std::string> >(result));
	}

	// we are done, return the result
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d devices", result.size());
	return result;
}

void	NiceLocator::check(const DeviceName& name,
		DeviceName::device_type type) {
	if (name.type() != type) {
		debug(LOG_ERR, DEBUG_LOG, 0, "name %s is not a camera",
			name.toString().c_str());
		throw std::runtime_error("name is not a camera");
	}
}

/**
 * \brief Get a wrapped camera
 */
CameraPtr	NiceLocator::getCamera0(const DeviceName& name) {
	check(name, DeviceName::Camera);

	// denice the camera name
	DeviceDenicer	denicer(name);

	// request a proxy for the camera with the deniced name
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);

	// wrap the proxy 
	snowstar::CameraPrx	camera
		= locator->getCamera(denicer.devicename().toString());
	return CameraPtr(new NiceCamera(camera, name));
}

CcdPtr	NiceLocator::getCcd0(const DeviceName& name) {
	check(name, DeviceName::Ccd);
	DeviceDenicer	denicer(name);
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);
	snowstar::CcdPrx	ccd
		= locator->getCcd(denicer.devicename().toString());
	return CcdPtr(new NiceCcd(ccd, name));
}

GuiderPortPtr	NiceLocator::getGuiderPort0(const DeviceName& name) {
	check(name, DeviceName::Guiderport);
	DeviceDenicer	denicer(name);
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);
	snowstar::GuiderPortPrx	guiderport
		= locator->getGuiderPort(denicer.devicename().toString());
	return GuiderPortPtr(new NiceGuiderPort(guiderport, name));
}

FilterWheelPtr	NiceLocator::getFilterWheel0(const DeviceName& name) {
	check(name, DeviceName::Filterwheel);
	DeviceDenicer	denicer(name);
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);
	snowstar::FilterWheelPrx	filterwheel
		= locator->getFilterWheel(denicer.devicename().toString());
	return FilterWheelPtr(new NiceFilterWheel(filterwheel, name));
}

CoolerPtr	NiceLocator::getCooler0(const DeviceName& name) {
	check(name, DeviceName::Cooler);
	DeviceDenicer	denicer(name);
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);
	snowstar::CoolerPrx	cooler
		= locator->getCooler(denicer.devicename().toString());
	return CoolerPtr(new NiceCooler(cooler, name));
}

FocuserPtr	NiceLocator::getFocuser0(const DeviceName& name) {
	check(name, DeviceName::Focuser);
	DeviceDenicer	denicer(name);
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);
	snowstar::FocuserPrx	focuser
		= locator->getFocuser(denicer.devicename().toString());
	return FocuserPtr(new NiceFocuser(focuser, name));
}

AdaptiveOpticsPtr	NiceLocator::getAdaptiveOptics0(const DeviceName& name) {
	check(name, DeviceName::AdaptiveOptics);
	DeviceDenicer	denicer(name);
	std::string	modulename = denicer.devicename().modulename();
	snowstar::DeviceLocatorPrx	locator = getLocator(denicer.service(),
						modulename);
	snowstar::AdaptiveOpticsPrx	adaptiveoptics
		= locator->getAdaptiveOptics(denicer.devicename().toString());
	return AdaptiveOpticsPtr(new NiceAdaptiveOptics(adaptiveoptics, name));
}

void	NiceLocator::ignoreservice(const std::string& service) {
	ignoredservices.insert(service);
}

} // namespace nice
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::nice::NiceLocator();
}
