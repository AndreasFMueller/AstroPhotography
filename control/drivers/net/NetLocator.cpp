/*
 * NetLocator.cpp -- CORBA based locator class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <NetLocator.h>
#include <AstroLoader.h>
#include <Conversions.h>
#include <OrbSingleton.h>
#include <AstroExceptions.h>
#include <AstroDevice.h>
#include <NetCamera.h>
#include <NetGuiderPort.h>
#include <NetFilterWheel.h>
#include <NetCooler.h>
#include <NetCcd.h>
#include <NetFocuser.h>
#include <NetUtils.h>
#include <DeviceLocatorAdapter.h>

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
	// get a reference to the CORBA server
	Astro::OrbSingleton	orbsingleton;
	modules = orbsingleton.getModules();
}

NetLocator::~NetLocator() {
	// release the CORBA reference
}

std::string	NetLocator::getName() const {
	return std::string("net");
}

std::string	NetLocator::getVersion() const {
	return VERSION;
}

std::string	NetLocator::modulename(const std::string& netname) const {
	if (netname.substr(0, 4) != "net:") {
		throw NotFound("not a net camera name");
	}
	std::string	purename = netname.substr(4);
	// locate the /, and return the part before the /
	size_t	offset = purename.find('/');
	if (offset == std::string::npos) {
		throw std::runtime_error("no / in name");
	}
	return purename.substr(0, offset);
}

std::string	NetLocator::devicename(const DeviceName& netname) const {
	if (netname.modulename() != "net") {
		throw NotFound("not a net device name");
	}
	return URL::decode(netname.unitname());
}

/**
 * \brief Retrieve a list of names of all objects of a given type
 *
 * This method collects devices from all remote modules, and encodes
 * module and device name in the single net device name
 */
std::vector<std::string>	NetLocator::getDevicelist(
	DeviceName::device_type device) {
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

		// query the descriptor
		Astro::Descriptor	*descriptor
			= drivermodule->getDescriptor();
		Astro::Descriptor_var	descriptorvar = descriptor;

		// if the driver module does not have we don't even try
		if (descriptor->hasDeviceLocator) {

			// get the device locator for this module
			Astro::DeviceLocator_ptr	devicelocator
				= drivermodulevar->getDeviceLocator();
			Astro::DeviceLocator_var	devcelocatorvar
				= devicelocator;

			// get a list of device names for this type
			Astro::DeviceLocator::DeviceNameList	*list
				= devicelocator->getDevicelist(type);
			Astro::DeviceLocator::DeviceNameList_var	listvar
				= list;

			// build new names from the name list received
			for (unsigned int j = 0; j < listvar->length(); j++) {
				char	*lp = (*list)[j];
				DeviceName	devname(lp);
				result.push_back(devname2netname(devname));
			}
		}
	}

	return result;
}

/**
 * \brief Get the device locator
 */
Astro::DeviceLocator_var	NetLocator::devicelocator(
	const DeviceName& netname) {
	std::string	modname = netname.modulename();

	// get the driver module
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve module %s", modname.c_str());
	Astro::DriverModule_ptr	drivermodule
		= modules->getModule(modname.c_str());
	Astro::DriverModule_var	drivermodulevar = drivermodule;

	// get the device locator
	Astro::DeviceLocator_ptr	devicelocator
		= drivermodulevar->getDeviceLocator();
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator;
	return devicelocatorvar;
}

#if 1

template<typename DeviceType, typename Type, typename NetType>
class NetLocatorHelper {
	Astro::Modules_var&	_modules;
public:
	NetLocatorHelper(Astro::Modules_var& modules) : _modules(modules) { }
	typename DeviceType::sharedptr	get(const DeviceName& name);
};

template<typename DeviceType, typename Type, typename NetType>
typename DeviceType::sharedptr
NetLocatorHelper<DeviceType, Type, NetType>::get(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for %s", sname.c_str());

	std::string	modname = name.modulename();

	// get the driver module
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve module %s", modname.c_str());
	Astro::DriverModule_ptr	drivermodule
		= _modules->getModule(modname.c_str());
	Astro::DriverModule_var	drivermodulevar = drivermodule;

	// get the device locator
	Astro::DeviceLocator_ptr	devicelocator
		= drivermodulevar->getDeviceLocator();
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator;

	// now create a device locator adapter from the locator var
	Astro::DeviceLocatorAdapter<typename Type::_ptr_type>
		devloc(devicelocatorvar);

	// get the device reference
	std::string	devname = URL::decode(name.unitname());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve %s", devname.c_str());
	typename Type::_var_type	devicevar
		= devloc.get(devname.c_str());

	// wrap it in a CameraPtr object
	return typename DeviceType::sharedptr(new NetType(devicevar));
}

#endif

/**
 * \brief Get a camera by name
 */
CameraPtr	NetLocator::getCamera0(const DeviceName& name) {
	NetLocatorHelper<Camera, Astro::Camera, NetCamera> lh(modules);
	return	lh.get(name);
#if 0
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for camera %s", sname.c_str());
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator(name);

	// get the device reference
	std::string	devname = devicename(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve camera %s", devname.c_str());
	Astro::Camera_var	devicevar
		= devicelocatorvar->getCamera(devname.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a remote camera with %d ccds",
		devicevar->nCcds());

	// wrap it in a CameraPtr object
	return CameraPtr(new NetCamera(devicevar));
#endif
}

CcdPtr	NetLocator::getCcd0(const DeviceName& name) {
	NetLocatorHelper<Ccd, Astro::Ccd, NetCcd> lh(modules);
	return	lh.get(name);
}

/**
 * \brief Get a guiderport by name
 */
GuiderPortPtr	NetLocator::getGuiderPort0(const DeviceName& name) {
	NetLocatorHelper<GuiderPort, Astro::GuiderPort, NetGuiderPort>
		lh(modules);
	return	lh.get(name);
#if 0
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for guider port %s",
		sname.c_str());
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator(name);

	// get the device reference
	std::string	devname = devicename(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve camera %s", devname.c_str());
	Astro::GuiderPort_var	devicevar
		= devicelocatorvar->getGuiderPort(devname.c_str());

	// wrap it in a GuiderPortPtr object
	return GuiderPortPtr(new NetGuiderPort(devicevar));
#endif
}

/**
 * \brief Get a filterwheel by name
 */
FilterWheelPtr	NetLocator::getFilterWheel0(const DeviceName& name) {
	NetLocatorHelper<FilterWheel, Astro::FilterWheel, NetFilterWheel> lh(modules);
	return	lh.get(name);
#if 0
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for filter wheel %s",
		sname.c_str());
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator(name);

	// get the device reference
	std::string	devname = devicename(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve filter wheel %s",
		devname.c_str());
	Astro::FilterWheel_var	devicevar
		= devicelocatorvar->getFilterWheel(devname.c_str());

	// wrap it in a FilterWheelPtr object
	return FilterWheelPtr(new NetFilterWheel(devicevar));
#endif
}

/**
 * \brief Get a cooler by name
 */
CoolerPtr	NetLocator::getCooler0(const DeviceName& name) {
	NetLocatorHelper<Cooler, Astro::Cooler, NetCooler> lh(modules);
	return	lh.get(name);
#if 0
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for cooler %s",
		sname.c_str());
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator(name);

	// get the device reference
	std::string	devname = devicename(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve cooler %s", devname.c_str());
	Astro::Cooler_var	devicevar
		= devicelocatorvar->getCooler(devname.c_str());

	// wrap it in a CoolerPtr object
	return CoolerPtr(new NetCooler(devicevar));
#endif
}

/**
 * \brief Get a focuser by name
 */
FocuserPtr	NetLocator::getFocuser0(const DeviceName& name) {
	NetLocatorHelper<Focuser, Astro::Focuser, NetFocuser> lh(modules);
	return	lh.get(name);
#if 0
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "request for focuser %s",
		sname.c_str());
	Astro::DeviceLocator_var	devicelocatorvar = devicelocator(name);

	// get the device reference
	std::string	devname = devicename(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve focuser %s", devname.c_str());
	Astro::Focuser_var	devicevar
		= devicelocatorvar->getFocuser(devname.c_str());

	// wrap it in a FocuserPtr object
	return FocuserPtr(new NetFocuser(devicevar));
#endif
}

} // namespace net
} // namespace module
} // namespace net

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::net::NetLocator();
}
