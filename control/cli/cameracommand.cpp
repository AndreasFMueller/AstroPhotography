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
#include <Cameras.h>

namespace astro {
namespace cli {

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
	"\tcamera <cameraid> assign <cameraname>\n"
	"\tcamera <cameraid> info\n"
	"\tcamera <cameraid> release\n"
	"\tcamera <cameraid> default\n"
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
	"The third synopsis tells the system that the camera with name\n"
	"<cameraid> is no longer needed.\n"
	"The fourth synopsis tells the system to use this camera as the\n"
	"default camera for all operations that need a camera\n"
	);
}

} // namespace cli
} // namespace astro
