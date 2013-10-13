/*
 * module.cpp -- module commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <modulecommand.h>
#include <AstroDebug.h>

namespace astro {
namespace cli {

void	modulecommand::operator()(const std::string& command,
		const std::vector<std::string>& arguments) {
	if (arguments.size() < 1) {
		throw command_error("module command requires arguments");
	}
	if (arguments[0] == std::string("help")) {
		help();
		return;
	}
	std::string	modulename = arguments[0];
	if (arguments[1] == std::string("version")) {
		moduleversion(modulename);
		return;
	}
	if (arguments[1] == std::string("camera")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module command");
		listdevices(modulename, Astro::DeviceLocator::DEVICE_CAMERA);
		return;
	}
	if (arguments[1] == std::string("focuser")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module command");
		listdevices(modulename, Astro::DeviceLocator::DEVICE_FOCUSER);
		return;
	}
	if (arguments[1] == std::string("guiderport")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module command");
		listdevices(modulename, Astro::DeviceLocator::DEVICE_GUIDERPORT);
		return;
	}
	if (arguments[1] == std::string("filterwheel")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module command");
		listdevices(modulename, Astro::DeviceLocator::DEVICE_FILTERWHEEL);
		return;
	}
	if (arguments[1] == std::string("cooler")) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "module command");
		listdevices(modulename, Astro::DeviceLocator::DEVICE_COOLER);
		return;
	}
	throw command_error("cannot execute module command");
}

void	modulecommand::listdevices(const std::string& modulename,
		const enum Astro::DeviceLocator::device_type devicetype) {
	// get the modules object
	guidesharedcli	gcli;
	Astro::Modules::ModuleNameSequence_var	namemodule
		= gcli->modules->getModuleNames();
	Astro::DriverModule_var drivermodule
		= gcli->modules->getModule(modulename.c_str());
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

void	modulecommand::help() {
	std::cout << "module command help" << std::endl;
	std::cout << "   module <modulename> camera" << std::endl;
	std::cout << "   module <modulename> focuser" << std::endl;
	std::cout << "   module <modulename> guiderport" << std::endl;
	std::cout << "   module <modulename> filterwheel" << std::endl;
	std::cout << "   module <modulename> cooler" << std::endl;
}

} // namespace cli
} // namespace astro
