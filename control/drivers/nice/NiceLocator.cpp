/*
 * NiceLocator.cpp -- Locator for net services
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NiceLocator.h>
#include <AstroLoader.h>
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <device.h>

namespace astro {
namespace camera {
namespace nice {

using namespace astro::discover;

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

std::vector<std::string>	NiceLocator::getDevicelist(
					DeviceName::device_type device,
					snowstar::DriverModulePrx module) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting devices from module %s",
		module->getName().c_str());
	std::vector<std::string>	result;
	snowstar::DeviceLocatorPrx locator = module->getDeviceLocator();
	snowstar::DeviceNameList	names
		= locator->getDevicelist(snowstar::convert(device));
	std::copy(names.begin(), names.end(),
		std::back_inserter<std::vector<std::string> >(result));
	return result;
}

std::vector<std::string>	NiceLocator::getDevicelist(
					DeviceName::device_type device,
					const ServiceKey& key) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting list from %s",
		key.toString().c_str());
	std::vector<std::string>	result;
	ServiceObject	object = discovery->find(key);

	// we need a connection 
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base
		= ic->stringToProxy(object.connect("Modules"));
	snowstar::ModulesPrx	modules
		= snowstar::ModulesPrx::checkedCast(base);

	snowstar::ModuleNameList	list = modules->getModuleNames();
	snowstar::ModuleNameList::const_iterator	i;
	for (i = list.begin(); i != list.end(); i++) {
		std::string	name = *i;
		if (name == "net") {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring net module");
			continue;
		}
		snowstar::DriverModulePrx	module = modules->getModule(*i);
		std::vector<std::string>	names
			= getDevicelist(device, module);
		std::copy(names.begin(), names.end(),
			std::back_inserter<std::vector<std::string> >(result));
	}

	result.push_back(object.host());
	return result;
}

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

CameraPtr	NiceLocator::getCamera0(const DeviceName& /* name */) {
	return CameraPtr();
}

CcdPtr	NiceLocator::getCcd0(const DeviceName& /* name */) {
	return CcdPtr();
}

GuiderPortPtr	NiceLocator::getGuiderPort0(const DeviceName& /* name */) {
	return GuiderPortPtr();
}

FilterWheelPtr	NiceLocator::getFilterWheel0(const DeviceName& /* name */) {
	return FilterWheelPtr();
}

CoolerPtr	NiceLocator::getCooler0(const DeviceName& /* name */) {
	return CoolerPtr();
}

FocuserPtr	NiceLocator::getFocuser0(const DeviceName& /* name */) {
	return FocuserPtr();
}

AdaptiveOpticsPtr	NiceLocator::getAdaptiveOptics0(const DeviceName& /* name */) {
	return AdaptiveOpticsPtr();
}

} // namespace nice
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::nice::NiceLocator();
}
