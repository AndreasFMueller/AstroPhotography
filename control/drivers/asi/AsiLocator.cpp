/*
 * AsiLocator.cpp -- camera locator for SBIG driver cameras
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
#include <includes.h>
#include <ASICamera2.h>
#include <utils.h>

//////////////////////////////////////////////////////////////////////
// Implementation of the SBIG Express Module Descriptor
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace module {
namespace asi {

static std::string      asi_name("asi");
static std::string      asi_version(VERSION);
static astro::camera::asi::AsiCameraLocator	*asi_locator = NULL;

/**
 * \brief Module descriptor for the SBIG module
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


int	AsiCameraLocator::driveropen = 0;

std::string	AsiCameraLocator::getName() const {
	return std::string("asi");
}

std::string	AsiCameraLocator::getVersion() const {
	return VERSION;
}

AsiCameraLocator::AsiCameraLocator() {
}

AsiCameraLocator::~AsiCameraLocator() {
}

static void	asiAddCameraNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		DeviceName	cameraname = asiCameraName(index);
		names.push_back(cameraname);
	}
}

static void	asiAddCcdNames(std::vector<std::string>& names ) {
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		DeviceName	ccdname = asiCcdName(index);
		names.push_back(ccdname);
	}
}

static void	asiAddGuiderportNames(std::vector<std::string>& names) {
	int	n = ASIGetNumOfConnectedCameras();
	for (int index = 0; index < n; index++) {
		DeviceName	guiderportname = asiGuiderportName(index);
		names.push_back(guiderportname);
	}
}

/**
 * \brief Get a list of SBIG cameras
 *
 * The cameras on the USB bus are number, that's the order in which the
 * locator returns the identifying string of the camera. A camera is
 * identified by its serial number an name.
 */
std::vector<std::string>	AsiCameraLocator::getDevicelist(DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get SBIG device list");
	std::vector<std::string>	names;
	switch (device) {
	case DeviceName::AdaptiveOptics:
	case DeviceName::Focuser:
	case DeviceName::Module:
	case DeviceName::Mount:
	case DeviceName::Filterwheel:
		return names;
	case DeviceName::Guiderport:
		asiAddGuiderportNames(names);
		return names;
	case DeviceName::Camera:
		asiAddCameraNames(names);
		return names;
	case DeviceName::Ccd:
		asiAddCcdNames(names);
		return names;
	default:
		break;
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
CameraPtr	AsiCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	size_t	index = 0;
	for (i = cameras.begin(); i != cameras.end(); i++, index++) {
		if (name == *i) {
//			return CameraPtr(new AsiCamera(index));
		}
	}
	std::string	msg = stringprintf("camera %s not found", sname.c_str());
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
