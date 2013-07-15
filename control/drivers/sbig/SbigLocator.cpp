/*
 * SbigLocator.cpp -- camera locator for SBIG driver cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigLocator.h>
#include <debug.h>
#include <sbigudrv.h>
#include <utils.h>
#include <Format.h>
#include <SbigCamera.h>
#include <includes.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

std::string	SbigCameraLocator::getName() const {
	return std::string("sbig");
}

std::string	SbigCameraLocator::getVersion() const {
	return VERSION;
}

SbigCameraLocator::SbigCameraLocator() {
	short	e = SBIGUnivDrvCommand(CC_OPEN_DRIVER, NULL, NULL);
	if (e != CE_NO_ERROR) {
		std::string	errmsg = sbig_error(e);
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot open driver: %s",
			errmsg.c_str());
		throw SbigError(errmsg.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "driver opened: %hd", e);
}

SbigCameraLocator::~SbigCameraLocator() {
	short	e = SBIGUnivDrvCommand(CC_CLOSE_DRIVER, NULL, NULL);
	if (e != CE_NO_ERROR) {
		std::string	errmsg = sbig_error(e);
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot close driver: %s",
			errmsg.c_str());
		throw SbigError(errmsg.c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "driver closed: %hd", e);
}

std::vector<std::string>	SbigCameraLocator::getCameralist() {
	std::vector<std::string>	names;
	QueryUSBResults	results;
	short	e = SBIGUnivDrvCommand(CC_QUERY_USB, NULL, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get camera list: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d cameras",
		results.camerasFound);
	for (int i = 0; i < results.camerasFound; i++) {
		if (results.usbInfo[i].cameraFound) {
			std::string	name = stringprintf("sbig:%s/%s",
				results.usbInfo[i].serialNumber,
				results.usbInfo[i].name);
			names.push_back(name);
		}
	}
	return names;
}

CameraPtr	SbigCameraLocator::getCamera(const std::string& name) {
	std::vector<std::string>	cameras = getCameralist();
	std::vector<std::string>::const_iterator	i;
	size_t	index = 0;
	for (i = cameras.begin(); i != cameras.end(); i++, index++) {
		if (name == *i) {
			return getCamera(index);
		}
	}
	std::string	msg = stringprintf("camera %s not found", name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

CameraPtr	SbigCameraLocator::getCamera(size_t index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening camera %d", index);
	return CameraPtr(new SbigCamera(index));
}

} // namespace sbig
} // namespace camera
} // namespace astro

extern "C"
astro::camera::CameraLocator	*getCameraLocator() {
	return new astro::camera::sbig::SbigCameraLocator();
}
