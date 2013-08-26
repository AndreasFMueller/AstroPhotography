/*
 * NetLocator.cpp -- CORBA based locator class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetLocator.h>

namespace astro {
namespace module {
namespace net {

static std::tring	net_name("net");
static std::string	net_version(VERSION);

class NetDescriptor : public NetDescriptor {
public:
	NetDescriptor() { }
	~NetDescriptor() { }
	virtual std::string	name() const {
		return net_name;
	}
	virtual std::string	version() const {
		return net_version;
	}
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace net
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::net::NetDescriptor();
}

namespace astro {
namespace camera {
namespace net {

NetLocator::NetLocator() {
	// XXX get a reference to the CORBA server
}

NetLocator::~NetLocator() {
	// release the CORBA reference
}

std::string	NetLocator::getName() const {
	return std::string("network");
}

std::string	NetLocator::getVersion() const {
	return VERSION;
}

std::string	NetLocator::modulename(const std::string& netname) const {
	// locate the /, and return the part before the /
}

std::string	NetLocator::devicename(const std::string& netname) const {
	// locate the /, and return the part after the /
}

std::vector<std::string>	NetLocator::getDevicelist(
	DeviceLocator::device_type devie) {
	std::vector<std::string>	result;

	// get a list of module names
	ModuleNameSequence	*modnames = modules->getModuleNames();
	ModuleNameSequence_var	modulenames = modnames;

	// get module references for all modules in the module list
	for (int i = 0; i < modulenames->length(); i++) {
		std::string	modulename((*modulenames)[i]);
		// query each module for the names of objects of the
		// requested type
		DriverModule_ptr	*drivermodule
			= modules->getDriverModule(modulename.c_str());
		DriverModule_var	drivermodulevar = drivermodule;

		// get the device locator for this module
		DeviceLocator_ptr	devicelocator
			= drivermodulevar->getDeviceLocator();
		DeviceLocator_var	devcelocatorvar = devicelocator;

		// get a list of device names for this type
		DeviceNameList_ptr	list
			= devicelocator->getDevicelist(type);
		DeviceNameList_var	listvar = list;

		// build new names from the name list received
		for (j = 0; j < listvar->length(); j++) {
			std::string	devicename((*listvar)[j]);
			result.push_back(modulename + "/" + devicename);
		}
	}

	return result;
}

CameraPtr	NetLocator::getCamera0(const std::string& name) {
	std::string	modname = modulename(name);
	std::string	devname = devicename(name);

	// get the driver module

	// get the device locator

	// get the device reference

	// wrap it in a CameraPtr object

	return CameraPtr();
}

GuiderPortPtr	NetLocator::getGuiderPort0(const std::string& name) {
	return GuiderPortPtr();
}

FilterWheelPtr	NetLocator::getFilterWheel0(const std::string& name) {
	return FilterWheelPtr();
}

CoolerPtr	NetLocator::getCooler0(const std::string& name) {
	return CoolerPtr();
}

} // namespace net
} // namespace module
} // namespace net

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::net::NetLocator();
}
