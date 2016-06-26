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

} // namespace asi
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::asi::AsiDescriptor();
}

using namespace astro;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace asi {

//////////////////////////////////////////////////////////////////////
// AsiLocator implementation
//////////////////////////////////////////////////////////////////////

std::vector<bool>	AsiCameraLocator::cameraopen;

std::string	AsiCameraLocator::getName() const {
	return std::string("asi");
}

std::string	AsiCameraLocator::getVersion() const {
	return VERSION;
}

void	AsiCameraLocator::initialize_cameraopen() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initialize the cameraopen array");
	int	n = ASIGetNumOfConnectedCameras();
	for (int i = 0; i < n; i++) {
		AsiCameraLocator::cameraopen[i] = false;
	}
}

std::once_flag	cameraopen_flag;

/**
 * \brief Create a new ASI camera locator
 */
AsiCameraLocator::AsiCameraLocator() {
	std::call_once(cameraopen_flag, initialize_cameraopen);
}

/**
 * \brief Destroy a new ASI camera locator
 */
AsiCameraLocator::~AsiCameraLocator() {
}

/**
 * \brief add the names of all cameras to a name vector
 *
 * \param names		list of names to which camera names should be added
 */
void	AsiCameraLocator::addCameraNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
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
 * \brief add the names of all guiderport to a name vector
 *
 * \param names		list of names to which guider port names should be added
 */
void	AsiCameraLocator::addGuiderportNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		DeviceName	guiderportname = asiGuiderportName(index);
		names.push_back(guiderportname);
	}
}

/**
 * \brief add the names of all cooler to a name vector
 *
 * \param names		list of names to which cooler names should be added
 */
void	AsiCameraLocator::addCoolerNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		std::vector<std::string>	it = imgtypes(index);
		std::vector<std::string>::const_iterator	i;
		for (i = it.begin(); i != it.end(); i++) {
			DeviceName	ccdname = asiCcdName(index, *i);
			names.push_back(ccdname.child(DeviceName::Cooler,
				"cooler"));
		}
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
std::vector<std::string>	AsiCameraLocator::getDevicelist(DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get ASI device list");
	std::vector<std::string>	names;
	switch (device) {
	case DeviceName::AdaptiveOptics:
	case DeviceName::Focuser:
	case DeviceName::Module:
	case DeviceName::Mount:
	case DeviceName::Filterwheel:
		return names;
	case DeviceName::Guiderport:
		addGuiderportNames(names);
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
	std::call_once(cameraopen_flag, initialize_cameraopen);
	if (index >= AsiCameraLocator::cameraopen.size()) {
		return true;
	}
	return cameraopen[index];
}

/**
 * \brief register that a camera is open
 *
 * \param index		index of the camera
 * \param o		whether or not the camera is open
 */
void	AsiCameraLocator::setopen(int index, bool o) {
	std::call_once(cameraopen_flag, initialize_cameraopen);
	cameraopen[index] = o;
}

/**
 * \brief Retrieve a list of image types 
 *
 * \param index		index of the camera 
 */
std::vector<std::string>	AsiCameraLocator::imgtypes(int index) {
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
        int     imgtypeidx = 0;
        while (camerainfo.SupportedVideoFormat[imgtypeidx] != -1) {
                std::string     it = AsiCcd::imgtype2string(
                        camerainfo.SupportedVideoFormat[imgtypeidx]);
		result.push_back(it);
	}
	if (!isopen(index)) {
		rc = ASICloseCamera(index);
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
 * \param name	name of the camera
 */
CameraPtr	AsiCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	size_t	index = 0;
	for (i = cameras.begin(); i != cameras.end(); i++, index++) {
		if (name == *i) {
			cameraopen[index] = true;
			return CameraPtr(new AsiCamera(index));
		}
	}
	std::string	msg = stringprintf("camera %s not found",
		sname.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
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
