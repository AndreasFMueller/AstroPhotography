/*
 * SbigLocator.cpp -- camera locator for SBIG driver cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SbigLocator.h>
#include <AstroDebug.h>
#include <sbigudrv.h>
#include <utils.h>
#include <AstroFormat.h>
#include <SbigCamera.h>
#include <includes.h>
#include <pthread.h>

using namespace astro;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

//////////////////////////////////////////////////////////////////////
// SbigLock implementation
//////////////////////////////////////////////////////////////////////

static pthread_mutex_t sbigmutex;

SbigLock::SbigLock() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking sbig mutex");
	pthread_mutex_lock(&sbigmutex);
}

SbigLock::~SbigLock() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unlocking sbig mutex");
	pthread_mutex_unlock(&sbigmutex);
}

//////////////////////////////////////////////////////////////////////
// SbigLocator implementation
//////////////////////////////////////////////////////////////////////

std::string	SbigCameraLocator::getName() const {
	return std::string("sbig");
}

std::string	SbigCameraLocator::getVersion() const {
	return VERSION;
}

SbigCameraLocator::SbigCameraLocator() {
	pthread_mutex_init(&sbigmutex, NULL);
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

/**
 * \brief Get a list of SBIG cameras
 *
 * The cameras on the USB bus are number, that's the order in which the
 * locator returns the identifying string of the camera. A camera is
 * identified by its serial number an name.
 */
std::vector<std::string>	SbigCameraLocator::getDevicelist(DeviceLocator::device_type device) {
	std::vector<std::string>	names;
	if (device != CAMERA) {
		return names;
	}
	QueryUSBResults	results;
	SbigLock	lock;
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

/**
 * \brief Get a camera by name
 *
 * This works by retrieving a list of cameras and the checking which number
 * has the right name. This index is then used to retreive the camera object
 * by number.
 */
CameraPtr	SbigCameraLocator::getCamera(const std::string& name) {
	std::vector<std::string>	cameras = getDevicelist();
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

/**
 * \brief Get a camera by number
 */
CameraPtr	SbigCameraLocator::getCamera(size_t index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening camera %d", index);
	return CameraPtr(new SbigCamera(index));
}

} // namespace sbig
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	return new astro::camera::sbig::SbigCameraLocator();
}
