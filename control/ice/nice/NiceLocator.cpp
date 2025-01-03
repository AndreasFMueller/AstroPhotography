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
#include <NiceGuidePort.h>
#include <NiceFocuser.h>
#include <NiceAdaptiveOptics.h>
#include <NiceFilterWheel.h>
#include <NiceCooler.h>
#include <NiceMount.h>

namespace astro {
namespace module {
namespace nice {

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

static std::once_flag   descriptor_once;
static astro::module::ModuleDescriptor  *descriptor;
void	setup_descriptor() {
	descriptor = new NiceDescriptor();
}

} // namespace nice
} // namespace camera
} // namespace astro

extern "C"
astro::module::ModuleDescriptor *getDescriptor() {
	std::call_once(astro::module::nice::descriptor_once,
		astro::module::nice::setup_descriptor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "NiceDescriptor: %p",
		astro::module::nice::descriptor);
	return astro::module::nice::descriptor;
}

namespace astro {
namespace camera {
namespace nice {

using namespace astro::discover;

//////////////////////////////////////////////////////////////////////
// NiceLocator class
//////////////////////////////////////////////////////////////////////

NiceLocator::NiceLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting service discovery for nice "
		"locator");
	discovery = ServiceDiscovery::get();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nice locator constructed");
}

NiceLocator::~NiceLocator() {
}

std::string	NiceLocator::getName() const {
	return astro::module::nice::nice_name;
}

std::string	NiceLocator::getVersion() const {
	return astro::module::nice::nice_version;
}

/**
 * \brief Get a handle to the modules on a server
 *
 * The NiceLocator class keeps a cache of modules proxies for each service
 * it has talked to.
 */
snowstar::ModulesPrx	NiceLocator::getModules(const std::string& servicename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a service named '%s'",
		servicename.c_str());
	std::unique_lock<std::recursive_mutex>	lock(modules_mtx);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "modules lock acquired");
	ModulesMap::iterator	i = modules.find(servicename);
	if (i != modules.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module for '%s' already known",
			servicename.c_str());
		return i->second;
	}

	// get a proxy to Modules
	debug(LOG_DEBUG, DEBUG_LOG, 0, "not known yet, discovering service '%s'",
		servicename.c_str());
	ServiceKey	key = discovery->waitfor(servicename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found service '%s'",
		key.toString().c_str());
	return getModules(key);
}

/**
 * \brief Get a modules proxy from a key
 *
 * \param key	service key for which to find the modules
 */
snowstar::ModulesPrx	NiceLocator::getModules(const ServiceKey& key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getModules(%s)",
		key.toString().c_str());
	std::unique_lock<std::recursive_mutex>	lock(modules_mtx);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "lock acquired, key = %s",
		key.toString().c_str());
	snowstar::ModulesPrx	mprx;
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get modules for key %s",
			key.toString().c_str());
		ServiceObject	object = discovery->find(key);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "discovery object found");

		// we need a connection 
		Ice::CommunicatorPtr	ic
			= snowstar::CommunicatorSingleton::get();
		std::string	connectstring = object.connect("Modules");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "connect string: '%s'",
			connectstring.c_str());
		Ice::ObjectPrx	base = ic->stringToProxy(connectstring);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "connecting to Modules: %p",
			base.get());
		mprx = snowstar::ModulesPrx::checkedCast(base);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got modules proxy");

		// store the new proxy in the modules map
		modules.insert(std::make_pair(key.name(), mprx));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "proxy added to map: %s",
			key.name().c_str());
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get a proxy: %s",
			x.what());
	}
	// here we have the new proxy
	return mprx;
}

/**
 * \brief Get a DriverModule proxy for a given module name
 *
 * \param servicename	service name
 * \param module	name of the module
 */
snowstar::DriverModulePrx	NiceLocator::getDriverModule(
	const std::string& servicename, const std::string& modulename) {
	snowstar::ModulesPrx	mprx = getModules(servicename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get module '%s' from service '%s'",
		modulename.c_str(), servicename.c_str());
	if (!mprx) {
		std::string	msg("no modules proxy (mdns resolution?)");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	snowstar::DriverModulePrx	dmprx = mprx->getModule(modulename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a driver module");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "driver module version: %s",
		dmprx->getVersion().c_str());
	return dmprx;
}

/** 
 * \brief Get a driver module to a service key and a module name
 *
 * \param key		service key
 * \param modulename	name of the module
 */
snowstar::DriverModulePrx	NiceLocator::getDriverModule(
	const ServiceKey& key, const std::string& modulename) {
	snowstar::ModulesPrx	mprx = getModules(key);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get module '%s' from service '%s'",
		modulename.c_str(), key.toString().c_str());
	if (!mprx) {
		std::string	msg("no modules proxy (mdns resolution?)");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	snowstar::DriverModulePrx	dmprx =  mprx->getModule(modulename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a driver module");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "driver module version: %s",
		dmprx->getVersion().c_str());
	return dmprx;
}

/**
 * \brief Get a DeviceLocator proxy for a given module name
 *
 * \param servicename	service name (essentially host name)
 * \param modulename	name of the module for which to get a locator
 */
snowstar::DeviceLocatorPrx	NiceLocator::getLocator(
	const std::string& servicename, const std::string& modulename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for %s@%s", modulename.c_str(),
		servicename.c_str());
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
 *
 * \param key		key for the service to get a locator for
 * \param modulename	module name of the module to get a locator
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
 *
 * \param device	type of device to list
 * \param module	module driving the device
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get a locator proxy for %s",
		module->getName().c_str());
	snowstar::DeviceLocatorPrx locator = module->getDeviceLocator();
	snowstar::DeviceNameList	names
		= locator->getDevicelist(snowstar::convert(device));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d names", names.size());
	std::copy(names.begin(), names.end(),
		std::back_inserter<std::vector<std::string> >(result));
	return result;
}

/**
 * \brief Get the dvice names from a given service
 *
 * \param device	type of device to list
 * \param key		key for the service to get devices from
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
		if ((name == "mock2") || (name == "nice"))
			continue;
		snowstar::DriverModulePrx	module = modules->getModule(*i);
		std::vector<std::string>	names
			= getDevicelist(device, module);
		names = astro::device::nice::DeviceNicer(key.name())(names);
		std::copy(names.begin(), names.end(),
			std::back_inserter<std::vector<std::string> >(result));
	}

	return result;
}

/**
 * \brief Get a list of all devices available through nice
 *
 * This means iterating through all the available services
 *
 * \param device	type of devices to list
 */
std::vector<std::string>	NiceLocator::getDevicelist(
					DeviceName::device_type device) {
	std::vector<std::string>	result;

	// getting a list of available severs
	ServiceDiscovery::ServiceKeySet	services = discovery->list();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of services: %d",
		services.size());
	ServiceDiscovery::ServiceKeySet::const_iterator	i;
	for (i = services.begin(); i != services.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "service %s",
			i->toString().c_str());
	}

	// got through the servers and 
	for (i = services.begin(); i != services.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"checking services service %s, name = '%s'",
			i->toString().c_str(), i->name().c_str());
		if (ServicePublisher::ispublished(i->name())) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "skip published '%s'",
				i->name().c_str());
			continue;
		}
		std::vector<std::string> names = getDevicelist(device, *i);
		std::copy(names.begin(), names.end(),
			std::back_inserter<std::vector<std::string> >(result));
	}

	// we are done, return the result
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d %s devices",
		result.size(),
		DeviceName::type2string(device).c_str());
	return result;
}

/**
 * \brief Check whether the name is of the right type
 *
 * \param name	name of the device
 * \param type	type of the device
 */
void	NiceLocator::check(const DeviceName& name,
		DeviceName::device_type type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %s is a %s?",
		name.toString().c_str(),
		DeviceName::type2string(type).c_str());
	if (!name.hasType(type)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "name %s is not a %s",
			name.toString().c_str(),
			DeviceName::type2string(type).c_str());
		throw std::runtime_error("name is not a camera");
	}
	if (name.isLocalDevice()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s is not a network device name",
			name.toString().c_str());
		throw std::logic_error("not a network device name");
	}
}

/**
 * \brief Get a wrapped camera
 */
CameraPtr	NiceLocator::getCamera0(const DeviceName& name) {
	check(name, DeviceName::Camera);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote camera name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::CameraPrx	camera
		= locator->getCamera(remotename.toString());
	CameraPtr	result(new NiceCamera(camera, name));
	return result;
}

CcdPtr	NiceLocator::getCcd0(const DeviceName& name) {
	check(name, DeviceName::Ccd);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote ccd name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::CcdPrx	ccd
		= locator->getCcd(remotename.toString());
	CcdPtr	result(new NiceCcd(ccd, name));
	return result;
}

GuidePortPtr	NiceLocator::getGuidePort0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for guide port");
	check(name, DeviceName::Guideport);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving nice guideport %s",
		name.toString().c_str());

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote guideport name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::GuidePortPrx	guideport
		= locator->getGuidePort(remotename.toString());
	GuidePortPtr	result(new NiceGuidePort(guideport, name));
	return result;
}

FilterWheelPtr	NiceLocator::getFilterWheel0(const DeviceName& name) {
	check(name, DeviceName::Filterwheel);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote filterwheel name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::FilterWheelPrx	filterwheel
		= locator->getFilterWheel(remotename.toString());
	FilterWheelPtr	result(new NiceFilterWheel(filterwheel, name));
	return result;
}

CoolerPtr	NiceLocator::getCooler0(const DeviceName& name) {
	check(name, DeviceName::Cooler);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote cooler name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::CoolerPrx	cooler
		= locator->getCooler(remotename.toString());
	CoolerPtr	result(new NiceCooler(cooler, name));
	return result;
}

FocuserPtr	NiceLocator::getFocuser0(const DeviceName& name) {
	check(name, DeviceName::Focuser);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote focuser name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::FocuserPrx	focuser
		= locator->getFocuser(remotename.toString());
	FocuserPtr	result(new NiceFocuser(focuser, name));
	return result;
}

AdaptiveOpticsPtr	NiceLocator::getAdaptiveOptics0(const DeviceName& name) {
	check(name, DeviceName::AdaptiveOptics);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote adaptive optics name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::AdaptiveOpticsPrx	adaptiveoptics
		= locator->getAdaptiveOptics(remotename.toString());
	AdaptiveOpticsPtr	result(new NiceAdaptiveOptics(adaptiveoptics, name));
	return result;
}

astro::device::MountPtr	NiceLocator::getMount0(const DeviceName& name) {
	check(name, DeviceName::Mount);

	astro::DeviceName	remotename = name.localdevice();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remote mount name: %s",
		remotename.toString().c_str());
	snowstar::DeviceLocatorPrx	locator = getLocator(name.servicename(),
						remotename.modulename());

	snowstar::MountPrx	mount
		= locator->getMount(remotename.toString());
	astro::device::MountPtr	result(new device::nice::NiceMount(mount, name));
	return result;
}

} // namespace nice
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::nice::NiceLocator();
}
