/*
 * NetLocator.cpp -- CORBA based locator class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetLocator.h>
#include <AstroLoader.h>
#include <device.hh>
#include <Conversions.h>

using namespace astro::device;

namespace astro {
namespace module {
namespace net {

static std::string	net_name("net");
static std::string	net_version(VERSION);

class NetDescriptor : public ModuleDescriptor {
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
	size_t	offset = netname.find('/');
	if (offset == std::string::npos) {
		throw std::runtime_error("no / in name");
	}
	return netname.substr(0, offset);
}

std::string	NetLocator::devicename(const std::string& netname) const {
	// locate the /, and return the part after the /
	size_t	offset = netname.find('/');
	if (offset == std::string::npos) {
		throw std::runtime_error("no / in name");
	}
	return netname.substr(offset + 1);
}

/**
 * \brief Retrieve a list of names of all objects of a given type
 */
std::vector<std::string>	NetLocator::getDevicelist(
	DeviceLocator::device_type device) {
	std::vector<std::string>	result;

	// convert the device_type to the corresponding CORBA type
	Astro::DeviceLocator::device_type	type = convert(device);

	// get a list of module names
	Astro::Modules::ModuleNameSequence	*modnames
		= modules->getModuleNames();
	Astro::Modules::ModuleNameSequence_var	modulenames = modnames;

	// get module references for all modules in the module list
	for (unsigned int i = 0; i < modulenames->length(); i++) {
		std::string	modulename((*modnames)[i]);
		// query each module for the names of objects of the
		// requested type
		Astro::DriverModule_ptr	drivermodule
			= modules->getModule(modulename.c_str());
		Astro::DriverModule_var	drivermodulevar = drivermodule;

		// get the device locator for this module
		Astro::DeviceLocator_ptr	devicelocator
			= drivermodulevar->getDeviceLocator();
		Astro::DeviceLocator_var	devcelocatorvar = devicelocator;

		// get a list of device names for this type
		Astro::DeviceLocator::DeviceNameList	*list
			= devicelocator->getDevicelist(type);
		Astro::DeviceLocator::DeviceNameList_var	listvar = list;

		// build new names from the name list received
		for (unsigned int j = 0; j < listvar->length(); j++) {
			std::string	devicename((*list)[j]);
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

} // namespace net
} // namespace module
} // namespace net

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::net::NetLocator();
}
