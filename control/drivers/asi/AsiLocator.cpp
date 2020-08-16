/*
 * AsiLocator.cpp -- camera locator for ASI driver cameras
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <AsiLocator.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AsiCamera.hh>
#include <AsiCcd.h>
#include <AsiCooler.h>
#include <includes.h>
#include <ASICamera2.h>
#include <utils.h>

//////////////////////////////////////////////////////////////////////
// Implementation of the ASI Express Module Descriptor
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace module {
namespace asi {

static std::string      asi_name("asi");
static std::string      asi_version(VERSION);
static astro::camera::asi::AsiCameraLocator	*asi_locator = NULL;

/**
 * \brief Module descriptor for the ASI module
 */
class AsiDescriptor : public ModuleDescriptor {
public:
	AsiDescriptor() { }
	~AsiDescriptor() { }
        virtual std::string     name() const {
                return asi_name;
        }
        virtual std::string     version() const {
                return asi_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

static std::once_flag   descriptor_once;
static astro::module::ModuleDescriptor  *descriptor;
void    setup_descriptor() {
        descriptor = new AsiDescriptor();
}

} // namespace asi
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	std::call_once(astro::module::asi::descriptor_once,
		astro::module::asi::setup_descriptor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "AsiDescriptor: %p",
		astro::module::asi::descriptor);
	return astro::module::asi::descriptor;
}

using namespace astro;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace asi {

//////////////////////////////////////////////////////////////////////
// AsiLocator implementation
//////////////////////////////////////////////////////////////////////

std::recursive_mutex	*AsiCameraLocator::_mutex;

std::vector<bool>	AsiCameraLocator::cameraopen;

std::string	AsiCameraLocator::getName() const {
	return std::string("asi");
}

std::string	AsiCameraLocator::getVersion() const {
	return VERSION;
}

/**
 * \brief Initialize the array of camera open flags
 *
 * The cameraopen array keeps track of which cameras have been opened.
 * It has to be initialize to a length corresponding to the number of
 * cameras available. All cameras are flagged as closed initially.
 */
void	AsiCameraLocator::locator_initialize() {
	// initialize the mutex
	_mutex = new std::recursive_mutex();

	// initialize the 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initialize the cameraopen array");
	int	n = ASIGetNumOfConnectedCameras();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d cameras", n);
	for (int i = 0; i < n; i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera %d (%d)", i,
			cameraopen.size());
		AsiCameraLocator::cameraopen.push_back(false);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"initalization of %d cameras complete", n);
}

std::once_flag	AsiCameraLocator::flag;

/**
 * \brief Create a new ASI camera locator
 */
AsiCameraLocator::AsiCameraLocator() {
	std::call_once(flag, locator_initialize);
}

/**
 * \brief Get the mutex
 */
std::recursive_mutex	*AsiCameraLocator::getMutex() {
	std::call_once(flag, locator_initialize);
	return _mutex;
}

/**
 * \brief Destroy a new ASI camera locator
 */
AsiCameraLocator::~AsiCameraLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy the locator");
	// close all cameras
	int	n = ASIGetNumOfConnectedCameras();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "closing %d cameras", n);
	for (int i = 0; i < n; i++) {
		if (isopen(i)) {
			ASICloseCamera(i);
			setopen(i, false);
		}
	}
}

/**
 * \brief add the names of all cameras to a name vector
 *
 * \param names		list of names to which camera names should be added
 */
void	AsiCameraLocator::addCameraNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving %d camera names", n);
	for (int index = 0; index < n; index++) {
		DeviceName	cameraname = asiCameraName(index);
		names.push_back(cameraname);
	}
}

/**
 * \brief add the names of all CCDs to a name vector
 *
 * \param names		list of names to which CCD names should be added
 */
void	AsiCameraLocator::addCcdNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving CCD names for %d cameras",
		n);
	for (int index = 0; index < n; index++) {
		std::vector<std::string>	it = imgtypes(index);
		std::vector<std::string>::const_iterator	i;
		for (i = it.begin(); i != it.end(); i++) {
			DeviceName	ccdname = asiCcdName(index, *i);
			names.push_back(ccdname);
		}
	}
}

/**
 * \brief add the names of all guideport to a name vector
 *
 * \param names		list of names to which guider port names should be added
 */
void	AsiCameraLocator::addGuideportNames(std::vector<std::string>& names) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving Guideport names");
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		DeviceName	guideportname = asiGuideportName(index);
		names.push_back(guideportname);
	}
}

/**
 * \brief add the names of all cooler to a name vector
 *
 * \param names		list of names to which cooler names should be added
 */
void	AsiCameraLocator::addCoolerNames(std::vector<std::string>& names) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving Cooler names");
	// XXX there is a bug here: we should really test for the existence
	// XXX of a cooler before we generate the name
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		DeviceName	coolername = asiCoolerName(index);
		names.push_back(coolername);
	}
}

/**
 * \brief Get a list of ASI cameras
 *
 * The cameras on the USB bus are number, that's the order in which the
 * locator returns the identifying string of the camera. A camera is
 * identified by its serial number an name.
 *
 * \param device	type of the devices to list
 */
std::vector<std::string>	AsiCameraLocator::getDevicelist(
		DeviceName::device_type device) {
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ASI device list");
	std::vector<std::string>	names;
	switch (device) {
	case DeviceName::AdaptiveOptics:
	case DeviceName::Focuser:
	case DeviceName::Module:
	case DeviceName::Mount:
	case DeviceName::Filterwheel:
		return names;
	case DeviceName::Guideport:
		addGuideportNames(names);
		return names;
	case DeviceName::Camera:
		addCameraNames(names);
		return names;
	case DeviceName::Ccd:
		addCcdNames(names);
		return names;
	case DeviceName::Cooler:
		addCoolerNames(names);
		return names;
	default:
		break;
	}
	return names;
}

/**
 * \brief Find out whether a camera is already open
 *
 * \param index		index of the camera
 */
bool	AsiCameraLocator::isopen(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "isopen(%d)", index);
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check camera %d", index);
	if (index >= AsiCameraLocator::cameraopen.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "out of range");
		return false;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d open: %s", index,
		(cameraopen[index]) ? "yes" : "no");
	return cameraopen[index];
}

/**
 * \brief register that a camera is open
 *
 * \param index		index of the camera
 * \param o		whether or not the camera is open
 */
void	AsiCameraLocator::setopen(int index, bool o) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setopen(%d) = %s",
		index, (o) ? "YES" : "NO");
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remember camera %d state %s", index,
		(o) ? "open" : "closed");
	cameraopen[index] = o;
}

/**
 * \brief Retrieve a list of image types 
 *
 * \param index		index of the camera 
 */
std::vector<std::string>	AsiCameraLocator::imgtypes(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving image types for %d", index);
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());
	// make sure the index is valid
	if (index >= ASIGetNumOfConnectedCameras()) {
		std::string	msg = stringprintf("");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	std::vector<std::string>	result;
	int	rc;
	if (!isopen(index)) {
		int	rc = ASIOpenCamera(index);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open camera %d: %d", index, rc);
		if (ASI_SUCCESS != rc) {
			std::string	msg = stringprintf("%d cannot open: %s",
				index, AsiCamera::error(rc).c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	ASI_CAMERA_INFO	camerainfo;
	if (ASI_SUCCESS != (rc = ASIGetCameraProperty(&camerainfo, index))) {
		std::string	msg = stringprintf("%d cannot get props: %s",
			index, AsiCamera::error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got camera info for %d", index);
        int     imgtypeidx = 0;
        while (camerainfo.SupportedVideoFormat[imgtypeidx] != -1) {
                std::string     it = AsiCcd::imgtype2string(
                        camerainfo.SupportedVideoFormat[imgtypeidx]);
		result.push_back(it);
		imgtypeidx++;
	}
	if (!isopen(index)) {
		rc = ASICloseCamera(index);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "close camera %d: %d",
			index, rc);
		if (ASI_SUCCESS != rc) {
			std::string	msg = stringprintf("%d cannot close: %s",
				index, AsiCamera::error(rc).c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
	}
	return result;
}

/**
 * \brief Get a camera by name
 *
 * This works by retrieving a list of cameras and the checking which number
 * has the right name. This index is then used to retreive the camera object
 * by number.
 *
 * \param name	name of the camera
 */
CameraPtr	AsiCameraLocator::getCamera0(const DeviceName& name) {
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());

	// locate a camera
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	size_t	index = 0;
	for (i = cameras.begin(); i != cameras.end(); i++, index++) {
		if (name == *i) {
			return CameraPtr(new AsiCamera(*this, index));
		}
	}
	std::string	msg = stringprintf("camera %s not found",
		sname.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get a Guideport by name
 *
 * \param name	the name of the guideport
 */
GuidePortPtr	AsiCameraLocator::getGuidePort0(const DeviceName& name) {
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());
	DeviceName	cameraname = name;
	cameraname.type(DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);
	if (camera->hasGuidePort()) {
		return camera->getGuidePort();
	}
	std::string msg = stringprintf("guideport %s not found",
		name.toString().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get a Cooler by name
 *
 * \param name	the name of the cooler
 */
CoolerPtr	AsiCameraLocator::getCooler0(const DeviceName& name) {
	std::lock_guard<std::recursive_mutex>	lock(*getMutex());
	DeviceName	cameraname = name;
	cameraname.type(DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);
	AsiCamera	*_camera = dynamic_cast<AsiCamera*>(&*camera);
	if (NULL == _camera) {
		std::string	msg = stringprintf("%s is not an asi camera",
			cameraname.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return _camera->getCooler();
}


} // namespace asi
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	if (NULL == astro::module::asi::asi_locator) {
		astro::module::asi::asi_locator
			= new astro::camera::asi::AsiCameraLocator();
	}
	return astro::module::asi::asi_locator;
}
