/*
 * Cameras.cpp -- camera reference repository implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Cameras.h>
#include <AstroDebug.h>
#include <camera.hh>
#include <OrbSingleton.h>
#include <DeviceMap.h>
#include <CorbaExceptionReporter.h>
#include <AstroDevice.h>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Class of internals for the cameras
//////////////////////////////////////////////////////////////////////

/**
 * \brief internals class for Camera repository
 */
class Camera_internals : public DeviceMap<Astro::Camera> {
public:
	Camera_internals() { }
	virtual void	assign(const std::string& cameraid,
				const std::vector<std::string>& arguments);
};

/**
 * \brief assign a camera to a name
 */
void	Camera_internals::assign(const std::string& cameraid,
		const std::vector<std::string>& arguments) {

	if (arguments.size() < 3) {
		throw devicemap_error("camera assign needs 3 arguments");
	}
	// extract module name
	std::string	cameraname = arguments[2];
	astro::DeviceName	devname(cameraname);
	std::string	modulename = devname.modulename();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera '%s' from module '%s'",
		cameraname.c_str(), modulename.c_str());

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

	// now ask the device locator for a camera with that name
	Astro::Camera_ptr	camera;
	try {
		camera = devicelocator->getCamera(cameraname.c_str());
	} catch (const CORBA::Exception& x) {
		std::string	s = Astro::exception2string(x);
		debug(LOG_ERR, DEBUG_LOG, 0, "getCamera exception: %s",
			s.c_str());
		throw std::runtime_error(s);
	}
	if (CORBA::is_nil(camera)) {
		throw devicemap_error("could not get camera");
	}

	// assign the Camera_var object to this 
	DeviceMap<Astro::Camera>::assign(cameraid, camera);
}


//////////////////////////////////////////////////////////////////////
// Cameras implementation
//////////////////////////////////////////////////////////////////////

Camera_internals	*Cameras::internals = NULL;

/**
 * \brief create the Cameras object
 */
Cameras::Cameras() {
	if (NULL == internals) {
		internals = new Camera_internals();
	}
}

CameraWrapper	Cameras::byname(const std::string& cameraid) {
	return internals->byname(cameraid);
}

void	Cameras::release(const std::string& cameraid) {
	internals->release(cameraid);
}

void	Cameras::assign(const std::string& cameraid,
			const std::vector<std::string>& arguments) {
	internals->assign(cameraid, arguments);
}

} // namespace cli
} // namespace astro
