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
#include <SbigFilterWheel.h>

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
	switch (device) {
	case DeviceName::AdaptiveOptics:
	case DeviceName::Focuser:
	case DeviceName::Module:
	case DeviceName::Mount:
		return names;
	default:
		break;
	}
	QueryUSBResults	results;
	SbigLock	lock;
	short	e = SBIGUnivDrvCommand(CC_QUERY_USB, NULL, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get camera list: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d cameras, looking for %s",
		results.camerasFound, DeviceName::type2string(device).c_str());
	for (int i = 0; i < results.camerasFound; i++) {
		if (results.usbInfo[i].cameraFound) {
			std::string	name(results.usbInfo[i].serialNumber);
			DeviceName	camname("sbig", name);
			switch (device) {
			case DeviceName::Camera:
				camname.type(device);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"adding camera %s",
					camname.toString().c_str());
				names.push_back(camname);
				break;
			case DeviceName::Guiderport:
				camname.type(device);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"adding guiderport %s",
					camname.toString().c_str());
				names.push_back(camname);
				break;
			case DeviceName::Ccd: {
				DeviceName	ccd(camname, device, "Imaging");
				names.push_back(ccd);
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"adding imaging ccd %s",
					ccd.toString().c_str());
				}
				switch (results.usbInfo[i].cameraType) {
				case ST7_CAMERA:
				case ST8_CAMERA:
				case ST5C_CAMERA:
				case TCE_CONTROLLER:
				case ST9_CAMERA:
				case ST10_CAMERA:
				case ST1K_CAMERA:
				case ST2K_CAMERA:
				case STL_CAMERA:
				case ST402_CAMERA:
				case STX_CAMERA:
				case ST4K_CAMERA:
				case STT_CAMERA:
				case STF_CAMERA: {
					DeviceName	gccd(camname, device,
						"Tracking");
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"adding guiding ccd %s",
						gccd.toString().c_str());
					names.push_back(gccd);
					}
					break;
				default:
					break;
				}
				break;
			case DeviceName::Cooler:
				switch (results.usbInfo[i].cameraType) {
				case ST7_CAMERA:
				case ST8_CAMERA:
				case ST5C_CAMERA:
				case TCE_CONTROLLER:
				case ST9_CAMERA:
				case ST10_CAMERA:
				case ST1K_CAMERA:
				case ST2K_CAMERA:
				case STL_CAMERA:
				case ST402_CAMERA:
				case STX_CAMERA:
				case ST4K_CAMERA:
				case STT_CAMERA:
				case STF_CAMERA: {
					DeviceName	cooler(camname, device,
						"Imaging");
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"adding cooler %s",
						cooler.toString().c_str());
					names.push_back(camname);
					}
					break;
				default:
					break;
				}
				break;
			case DeviceName::Filterwheel:
				switch (results.usbInfo[i].cameraType) {
				case STX_CAMERA:
					camname.type(device);
					debug(LOG_DEBUG, DEBUG_LOG, 0,
						"adding filterwheel %s",
						camname.toString().c_str());
					names.push_back(camname);
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "device found: %s",
				camname.toString().c_str());
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

/**
 * \brief Get a filterwheel by name
 *
 * This function retrieves the filterwheel attached to a camera.
 *
 * \param name	name of the filterwheel
 */
FilterWheelPtr	SbigCameraLocator::getFilterWheel0(const DeviceName& name) {
	// get the corresponding camera name
	DeviceName	cameraname = name;
	cameraname.type(DeviceName::Camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for camera %s",
		cameraname.toString().c_str());

	// get the list of cameras
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		if (cameraname == *i) {
			CameraPtr	camera = getCamera(cameraname);
			SbigCamera	*sbigcam = dynamic_cast<SbigCamera *>(&*camera);
			return FilterWheelPtr(new SbigFilterWheel(*sbigcam));
		}
	}
	std::string	msg = stringprintf("filterhweel %s not found",
				name.toString().c_str());
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
