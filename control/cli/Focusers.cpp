/*
 * Focusers.cpp -- focuser reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Focusers.h>
#include <AstroDebug.h>
#include <camera.hh>
#include <OrbSingleton.h>
#include <DeviceMap.h>
#include <CorbaExceptionReporter.h>
#include <AstroDevice.h>
#include <stdexcept>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Class of internals for the focusers
//////////////////////////////////////////////////////////////////////

/**
 * \brief internals class for Focuser repository
 */
class Focuser_internals : public DeviceMap<Astro::Focuser> {
public:
	Focuser_internals() { }
	virtual void	assign(const std::string& focuserid,
				const std::vector<std::string>& arguments);
};

/**
 * \brief assign a focuser to a name
 */
void	Focuser_internals::assign(const std::string& focuserid,
		const std::vector<std::string>& arguments) {

	if (arguments.size() < 3) {
		throw devicemap_error("focuser assign needs 3 arguments");
	}
	// extract module name
	std::string	focusername = arguments[2];
	astro::DeviceName	devname(focusername);
	std::string	modulename = devname.modulename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get focuser '%s' from module '%s'",
		focusername.c_str(), modulename.c_str());

	// geht the modules interface
	Astro::OrbSingleton	orb;
	Astro::Modules_var	modules;
	try {
		modules = orb.getModules();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x).c_str();
		debug(LOG_ERR, DEBUG_LOG, 0, "getModules() exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}

	// get the driver module
	Astro::DriverModule_var	drivermodule;
	try {
		drivermodule = modules->getModule(modulename.c_str());
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getModule exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(drivermodule)) {
		throw devicemap_error("could not get module");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got driver module");

	// get the device locator
	Astro::DeviceLocator_var	devicelocator;
	try {
		devicelocator = drivermodule->getDeviceLocator();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getDeviceLocator exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(devicelocator)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get device locator");
		throw std::runtime_error("cannot get device locator");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got device locator for %s",
		modulename.c_str());

	// now ask the device locator for a focuser with that name
	Astro::Focuser_ptr	focuser;
	try {
		focuser = devicelocator->getFocuser(focusername.c_str());
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getFocuser exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(focuser)) {
		throw devicemap_error("could not get focuser");
	}

	// assign the Focuser_var object to this 
	DeviceMap<Astro::Focuser>::assign(focuserid, focuser);
}


//////////////////////////////////////////////////////////////////////
// Focusers implementation
//////////////////////////////////////////////////////////////////////

Focuser_internals	*Focusers::internals = NULL;

/**
 * \brief create the Focusers object
 */
Focusers::Focusers() {
	if (NULL == internals) {
		internals = new Focuser_internals();
	}
}

FocuserWrapper	Focusers::byname(const std::string& focuserid) {
	return internals->byname(focuserid);
}

void	Focusers::release(const std::string& focuserid) {
	internals->release(focuserid);
}

void	Focusers::assign(const std::string& focuserid,
			const std::vector<std::string>& arguments) {
	internals->assign(focuserid, arguments);
}

} // namespace cli
} // namespace astro
