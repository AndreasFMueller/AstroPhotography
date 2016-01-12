/*
 * SbigLocator.cpp -- camera locator for SBIG driver cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LPARDRV_H
#include <lpardrv.h>
#else
#ifdef HAVE_SBIGUDRV_LPARDRV_H
#include <SBIGUDrv/lpardrv.h>
#endif /* HAVE_SBIGUDRV_LPARDRV_H */
#endif

#include <SbigLocator.h>
#include <AstroDebug.h>
#include <utils.h>
#include <AstroFormat.h>
#include <SbigCamera.h>
#include <includes.h>

//////////////////////////////////////////////////////////////////////
// Implementation of the SBIG Express Module Descriptor
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace module {
namespace sbig {

static std::string      sbig_name("sbig");
static std::string      sbig_version(VERSION);
static astro::camera::sbig::SbigCameraLocator	*sbig_locator = NULL;

/**
 * \brief Module descriptor for the SBIG module
 */
class SbigDescriptor : public ModuleDescriptor {
public:
	SbigDescriptor() { }
	~SbigDescriptor() { }
        virtual std::string     name() const {
                return sbig_name;
        }
        virtual std::string     version() const {
                return sbig_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace sbig
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::sbig::SbigDescriptor();
}

using namespace astro;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {


//////////////////////////////////////////////////////////////////////
// SbigLock implementation
//////////////////////////////////////////////////////////////////////

static std::recursive_mutex	sbigmutex;
static std::unique_lock<std::recursive_mutex>	sbiglock(sbigmutex, std::defer_lock);

SbigLock::SbigLock() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locking sbig mutex");
	sbigmutex.lock();
}

SbigLock::~SbigLock() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unlocking sbig mutex");
	sbigmutex.unlock();
}

//////////////////////////////////////////////////////////////////////
// SbigLocator implementation
//////////////////////////////////////////////////////////////////////


int	SbigCameraLocator::driveropen = 0;

std::string	SbigCameraLocator::getName() const {
	return std::string("sbig");
}

std::string	SbigCameraLocator::getVersion() const {
	return VERSION;
}

SbigCameraLocator::SbigCameraLocator() {
	if (0 == driveropen) {
		short	e = SBIGUnivDrvCommand(CC_OPEN_DRIVER, NULL, NULL);
		if (e != CE_NO_ERROR) {
			std::string	errmsg = sbig_error(e);
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot open driver: %s",
				errmsg.c_str());
			throw SbigError(errmsg.c_str());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driver opened: %hd", e);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driver already open");
	}
	driveropen++;
}

SbigCameraLocator::~SbigCameraLocator() {
	if (0 == --driveropen) {
		short	e = SBIGUnivDrvCommand(CC_CLOSE_DRIVER, NULL, NULL);
		if (e != CE_NO_ERROR) {
			std::string	errmsg = sbig_error(e);
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot close driver: %s",
				errmsg.c_str());
			throw SbigError(errmsg.c_str());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driver closed: %hd", e);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d remaining driver references",
			driveropen);
	}
}

/**
 * \brief Get a list of SBIG cameras
 *
 * The cameras on the USB bus are number, that's the order in which the
 * locator returns the identifying string of the camera. A camera is
 * identified by its serial number an name.
 */
std::vector<std::string>	SbigCameraLocator::getDevicelist(DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get SBIG device list");
	std::vector<std::string>	names;
	if (device != DeviceName::Camera) {
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
			std::string	name = stringprintf("%s",
				results.usbInfo[i].serialNumber);
			DeviceName	camname("sbig", name);
			camname.type(DeviceName::Camera);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "device found: %s",
				name.c_str());
			names.push_back(camname);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "returning list with %d members",
		names.size());
	return names;
}

/**
 * \brief Get a camera by name
 *
 * This works by retrieving a list of cameras and the checking which number
 * has the right name. This index is then used to retreive the camera object
 * by number.
 */
CameraPtr	SbigCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	size_t	index = 0;
	for (i = cameras.begin(); i != cameras.end(); i++, index++) {
		if (name == *i) {
			return CameraPtr(new SbigCamera(index));
		}
	}
	std::string	msg = stringprintf("camera %s not found", sname.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace sbig
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	if (NULL == astro::module::sbig::sbig_locator) {
		astro::module::sbig::sbig_locator
			= new astro::camera::sbig::SbigCameraLocator();
	}
	return astro::module::sbig::sbig_locator;
}
