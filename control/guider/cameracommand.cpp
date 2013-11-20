/*
 * cameracommand.cpp -- camera command implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cameracommand.h>
#include <AstroDebug.h>
#include <map>
#include <camera.hh>
#include <OrbSingleton.h>
#include <iostream>
#include <ObjWrapper.h>
#include <DeviceMap.h>
#include <CorbaExceptionReporter.h>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Class of internals for the cameras
//////////////////////////////////////////////////////////////////////
typedef	ObjWrapper<Astro::Camera>	CameraWrapper;

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

	if (arguments.size() < 4) {
		throw command_error("camera assign needs 4 arguments");
	}
	// extract module name
	std::string	modulename = arguments[2];
	std::string	cameraname = arguments[3];
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
		throw command_error("could not get module");
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
		throw command_error("could not get camera");
	}

	// assign the Camera_var object to this 
	DeviceMap<Astro::Camera>::assign(cameraid, camera);
}


//////////////////////////////////////////////////////////////////////
// Cameras implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief class to mediate access to 
 */
class Cameras {
	static Camera_internals	*internals;
public:
	Cameras();
	CameraWrapper	byname(const std::string& cameraid);
	void	release(const std::string& cameraid);
	void	assign(const std::string& cameraid,
			const std::vector<std::string>& arguments);
};

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

//////////////////////////////////////////////////////////////////////
// cameracommand implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief release a camera 
 */
void	cameracommand::release(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera release subcommand");
	Cameras	cameras;
	cameras.release(cameraid);
}

std::ostream&	operator<<(std::ostream& out, Astro::CcdInfo *ccdinfo) {
	out << "name:        " << ccdinfo->name << std::endl;
	out << "id:          " << ccdinfo->id << std::endl;
	out << "size:        " << ccdinfo->size.width << " x "
				<< ccdinfo->size.height << std::endl;
	out << "shutter:     " << ((ccdinfo->shutter) ? "YES" : "NO")
		<< std::endl;
	out << "pixel size:  " << (1000000 * ccdinfo->pixelwidth) << " x "
		<< (1000000 * ccdinfo->pixelheight) << std::endl;
	return out;
}

/**
 * \brief display information about a camera available in the Cameras object
 */
void	cameracommand::info(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera info subcommand");
	Cameras	cameras;
	CameraWrapper	camera = cameras.byname(cameraid);
	std::cout << "name:           " << (*camera)->getName() << std::endl;
	std::cout << "number of ccds: " << (*camera)->nCcds() << std::endl;
	for (long ccdno = 0; ccdno < (*camera)->nCcds(); ccdno++) {
		Astro::CcdInfo	*info = (*camera)->getCcdinfo(ccdno);
		Astro::CcdInfo_var	v = info;
		std::cout << info;
	}
}

/**
 * \brief Assign a camera to a name
 */
void	cameracommand::assign(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera assign subcommand");
	Cameras	cameras;
	cameras.assign(cameraid, arguments);
}

/**
 * \brief execute a subcommand
 */
void	cameracommand::operator()(const std::string& commandname,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera command");

	if (arguments.size() < 2) {
		throw command_error("camera command requires 2 arguments");
	}

	std::string	cameraid = arguments[0];
	std::string	subcommandname = arguments[1];

	if (subcommandname == "info") {
		info(cameraid, arguments);
		return;
	}

	if (subcommandname == "release") {
		release(cameraid, arguments);
		return;
	}

	if (subcommandname == "assign") {
		assign(cameraid, arguments);
		return;
	}

	throw command_error("camera subcommand not known");

}

std::string	cameracommand::summary() const {
	return std::string("access cameras");
}

std::string	cameracommand::help() const {
	return std::string(
	"SYNOPSIS\n"
	"\n"
	"\tcamera <cameraid> assign <modulename> <cameraname>\n"
	"\tcamera <cameraid> info\n"
	"\tcamera <cameraid> release\n"
	"\n"
	"DESCRIPTION\n"
	"\n"
	"The camera command identifies cameras using a camera id, which\n"
	"is always given as the first argument of the command. The user is\n"
	"free to choose the camera id, but it should consist only of alpha-\n"
	"numeric characters.\n"
	"The second synopsis displays available information about a camera,\n"
	"including the list of CCDs available on that camera.\n"
	"The third synopsis makes the camera named <cameraname> from module\n"
	"<modulename> available as the camera with id <cameraid>.\n"
	"The second synopsis tells the system that the camera with name\n"
	"<cameraid> is no longer needed.\n"
	);
}

} // namespace cli
} // namespace astro
