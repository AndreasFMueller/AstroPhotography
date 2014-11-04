/*
 * module.cpp -- module commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <modulecommand.h>
#include <AstroDebug.h>
#include <CorbaExceptionReporter.h>

namespace astro {
namespace cli {

void	modulecommand::operator()(const std::string& /* command */,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		throw command_error("module command requires arguments");
	}

	std::string	modulename = arguments[0];
	if (arguments.size() < 2) {
		throw std::runtime_error("not enough arguments");
	}

	if (arguments[1] == std::string("version")) {
		moduleversion(modulename);
		return;
	}

	if (arguments[1] == std::string("list")) {
		if (arguments.size() < 3) {
			throw std::runtime_error("not enough arguments");
		}
		// initialize the type variable to keep the compiler from
		// complaining about unitialized variables
		Astro::DeviceLocator::device_type	type
			= Astro::DeviceLocator::DEVICE_CAMERA;
		if (arguments[2] == std::string("camera")) {
			type = Astro::DeviceLocator::DEVICE_CAMERA;
		} else if (arguments[2] == std::string("focuser")) {
			type = Astro::DeviceLocator::DEVICE_FOCUSER;
		} else if (arguments[2] == std::string("guiderport")) {
			type = Astro::DeviceLocator::DEVICE_GUIDERPORT;
		} else if (arguments[2] == std::string("filterwheel")) {
			type = Astro::DeviceLocator::DEVICE_FILTERWHEEL;
		} else if (arguments[2] == std::string("cooler")) {
			type = Astro::DeviceLocator::DEVICE_COOLER;
		} else {
			std::runtime_error("unknown device catogory");
		}
		listdevices(modulename, type);
		return;
	}
	throw command_error("cannot execute module command");
}

void	modulecommand::listdevices(const std::string& modulename,
		const enum Astro::DeviceLocator::device_type devicetype) {
	// get the modules object
	guidesharedcli	gcli;

	// get the module names available
	Astro::Modules::ModuleNameSequence_var	namemodule;
	try {
		namemodule = gcli->modules->getModuleNames();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getModuleNames exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	// get a module of a given name
	Astro::DriverModule_var drivermodule;
	try {
		drivermodule = gcli->modules->getModule(modulename.c_str());
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getModule exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	// get the device locator from the module
	Astro::DeviceLocator_var        devicelocator;
	try {
		devicelocator = drivermodule->getDeviceLocator();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getDeviceLocator exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	// get the list of device names
	Astro::DeviceLocator::DeviceNameList_var	namelist;
	try {
		namelist = devicelocator->getDevicelist(devicetype);
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getDevicelist exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	for (int i = 0; i < (int)namelist->length(); i++) {
		std::cout << namelist[i] << std::endl;
	}

}

void	modulecommand::moduleversion(const std::string& modulename) {
	guidesharedcli	gcli;
	Astro::Modules::ModuleNameSequence_var	namemodule
		= gcli->modules->getModuleNames();
	Astro::DriverModule_var drivermodule
		= gcli->modules->getModule(modulename.c_str());
	Astro::DeviceLocator_var        devicelocator
		= drivermodule->getDeviceLocator();
	std::cout << "module: " << devicelocator->getName() << ", version: " <<
		devicelocator->getVersion() << std::endl;
}

std::string	modulecommand::summary() const {
	return std::string("manipulate driver modules");
}

std::string	modulecommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tmodule <modulename> version\n"
	"\tmodule <modulename> list [ camera | focuser | guiderport | filterwheel | cooler ]\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The first synopsis displays version information about the module.\n"
	"The second synopsis List objects of a given type from the module\n"
	"named <modulename>.\n"
	);
}

} // namespace cli
} // namespace astro
