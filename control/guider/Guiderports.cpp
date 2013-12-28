/*
 * Guiderports.cpp -- guider port reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guiderports.h>
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
// Class of internals for the guiderports
//////////////////////////////////////////////////////////////////////

/**
 * \brief internals class for Camera repository
 */
class GuiderPort_internals : public DeviceMap<Astro::GuiderPort> {
public:
	GuiderPort_internals() { }
	virtual void	assign(const std::string& guiderportid,
				const std::vector<std::string>& arguments);
};

/**
 * \brief assign a guiderport to a name
 */
void	GuiderPort_internals::assign(const std::string& guiderportid,
		const std::vector<std::string>& arguments) {

	if (arguments.size() < 3) {
		throw devicemap_error("guiderport assign needs 3 arguments");
	}
	// extract module name
	std::string	guiderportname = arguments[2];
	astro::DeviceName	devname(guiderportname);
	std::string	modulename = devname.modulename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get guiderport '%s' from module '%s'",
		guiderportname.c_str(), modulename.c_str());

	// geht the modules interface
	Astro::OrbSingleton	orb;
	Astro::Modules_var	modules;
	try {
		modules = orb.getModules();
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x)/*.c_str()*/;
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

	// now ask the device locator for a guiderport with that name
	Astro::GuiderPort_ptr	guiderport;
	try {
		guiderport = devicelocator->getGuiderPort(guiderportname.c_str());
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getGuiderport exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(guiderport)) {
		throw devicemap_error("could not get guiderport");
	}

	// assign the GuiderPort_var object to this 
	DeviceMap<Astro::GuiderPort>::assign(guiderportid, guiderport);
}


//////////////////////////////////////////////////////////////////////
// Guiderports implementation
//////////////////////////////////////////////////////////////////////

GuiderPort_internals	*Guiderports::internals = NULL;

/**
 * \brief create the Guiderports object
 */
Guiderports::Guiderports() {
	if (NULL == internals) {
		internals = new GuiderPort_internals();
	}
}

GuiderPortWrapper	Guiderports::byname(const std::string& guiderportid) {
	return internals->byname(guiderportid);
}

void	Guiderports::release(const std::string& guiderportid) {
	internals->release(guiderportid);
}

void	Guiderports::assign(const std::string& guiderportid,
			const std::vector<std::string>& arguments) {
	internals->assign(guiderportid, arguments);
}

} // namespace cli
} // namespace astro
