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
#include <VarWrapper.h>

namespace astro {
namespace cli {

//////////////////////////////////////////////////////////////////////
// Class of internals for the cameras
//////////////////////////////////////////////////////////////////////
typedef	ObjWrapper<Astro::Camera>	CameraWrapper;

class Camera_internals /* : std::map<std::string, Astro::Camera_ptr> */ {
	typedef	std::map<std::string, Astro::Camera_ptr>	cameramap_t;
	cameramap_t	m;
public:
	Camera_internals() { }
	CameraWrapper	byname(const std::string& cameraid);
	void	release(const std::string& cameraid);
	void	assign(const std::string& cameraid,
				const std::vector<std::string>& arguments);
};

static Camera_internals	*commoncameras = NULL;

CameraWrapper	Camera_internals::byname(const std::string& cameraid) {
	cameramap_t::iterator	i = m.find(cameraid);
	if (i == m.end()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera %s not found",
			cameraid.c_str());
		throw command_error("camera not found");
	}
	return i->second;
}

void	Camera_internals::release(const std::string& cameraid) {
	cameramap_t::iterator	i = m.find(cameraid);
	if (i != m.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing camera %s",
			cameraid.c_str());
		m.erase(i);
	}
}

void	Camera_internals::assign(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
#if 1
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
	Astro::Modules_var	modules = orb.getModules();

	// get the driver module
	Astro::DriverModule_var	drivermodule
		= modules->getModule(modulename.c_str());
	if (CORBA::is_nil(drivermodule)) {
		throw command_error("could not get module");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got driver module");

	// get the device locator
	Astro::DeviceLocator_var	devicelocator
		= drivermodule->getDeviceLocator();
	if (CORBA::is_nil(devicelocator)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get device locator");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got device locator for %s",
		modulename.c_str());

	// now ask the device locator for a camera with that name
	Astro::Camera_ptr	camera
		= devicelocator->getCamera(cameraname.c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera ptr: %p", camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "int ptr: %p", new int);
	if (CORBA::is_nil(camera)) {
		throw command_error("could not get camera");
	}

	//CameraWrapper	cw(camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got camera '%s'", cameraname.c_str());

	// assign the Camera_var object to this 
	cameramap_t::value_type	v(cameraid, camera);
	m.insert(v);
#endif

#if 0
	std::pair<std::string, std::shared_ptr<int> >	v(cameraid, std::shared_ptr<int>(new int));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new pair at %p", &v);
	m.insert(v);
#endif
	//m.insert(std::make_pair(cameraid, camera));
	//m.insert(std::make_pair(cameraid, NULL));

//	std::map<std::string, int *>	m;
//	m.insert(std::make_pair(std::string("blubb"), (int *)NULL));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera reference stored in map");
}


//////////////////////////////////////////////////////////////////////
// Cameras implementation
//////////////////////////////////////////////////////////////////////
class Cameras {
	Camera_internals	*internals;
public:
	Cameras();
	CameraWrapper	byname(const std::string& cameraid);
	void	release(const std::string& cameraid);
	void	assign(const std::string& cameraid,
			const std::vector<std::string>& arguments);
};

Cameras::Cameras() {
	if (commoncameras) {
		internals = commoncameras;
	} else {
		commoncameras = new Camera_internals();
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

void	cameracommand::release(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera release subcommand");
	Cameras	cameras;
	cameras.release(cameraid);
}

void	cameracommand::info(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera info subcommand");
	Cameras	cameras;
	CameraWrapper	camera = cameras.byname(cameraid);
//	std::cout << "name: " << camera->getName() << std::endl;
}

void	cameracommand::assign(const std::string& cameraid,
		const std::vector<std::string>& arguments) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera assign subcommand");
	Cameras	cameras;
	cameras.assign(cameraid, arguments);
}

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
