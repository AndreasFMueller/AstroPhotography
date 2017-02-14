/*
 * AtikLocator.cpp -- camera locator for Atik driver cameras
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <atikccdusb.h>
#include <AtikLocator.h>
#include <AtikUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AtikCamera.h>
#include <includes.h>
#include <mutex>

//////////////////////////////////////////////////////////////////////
// Implementation of the Atik Module Descriptor
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace module {
namespace atik {

static std::string      atik_name("atik");
static std::string      atik_version(VERSION);
static astro::camera::atik::AtikCameraLocator	*atik_locator = NULL;

/**
 * \brief Module descriptor for the Atik module
 */
class AtikDescriptor : public ModuleDescriptor {
public:
	AtikDescriptor() { }
	~AtikDescriptor() { }
        virtual std::string     name() const {
                return atik_name;
        }
        virtual std::string     version() const {
                return atik_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace atik
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::atik::AtikDescriptor();
}

using namespace astro;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace atik {

//////////////////////////////////////////////////////////////////////
// AtikLocator implementation
//////////////////////////////////////////////////////////////////////

std::string	AtikCameraLocator::getName() const {
	return std::string("atik");
}

std::string	AtikCameraLocator::getVersion() const {
	return VERSION;
}

#define MaxAtikCameraNumber	10
static ::AtikCamera	*atik_camera[MaxAtikCameraNumber];
static int	atik_camera_count;
std::once_flag	atik_camera_flag;
void	atik_list_cameras() {
	atik_camera_count = ::AtikCamera::list((::AtikCamera**)atik_camera,
		MaxAtikCameraNumber);
	for (int i = 0; i < atik_camera_count; i++) {
		bool	rc = atik_camera[i]->open();
		if (!rc) {
			std::string	msg("cannot open camera");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ATIK cameras",
		atik_camera_count);
}

/**
 * \brief Create a new Atik camera locator
 */
AtikCameraLocator::AtikCameraLocator() {
	std::call_once(atik_camera_flag,atik_list_cameras);
}

/**
 * \brief Destroy a new Atik camera locator
 */
AtikCameraLocator::~AtikCameraLocator() {
}


/**
 * \brief Get a list of Atik cameras
 *
 * \param device	type of the devices to list
 */
std::vector<std::string>	AtikCameraLocator::getDevicelist(DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get Atik device list");
	std::vector<std::string>	names;
	// return an empty list for all the device types that we don't support
	switch (device) {
	case DeviceName::AdaptiveOptics:
	case DeviceName::Focuser:
	case DeviceName::Module:
	case DeviceName::Mount:
		return names;
	default:
		// default: all device types that the atik driver can handle
		break;
	}
	// now go through the cameras and collect information about them
	for (int i = 0; i < atik_camera_count; i++) {
		// retrieve the capabilities
		struct AtikCapabilities	capa;
		const char	*name;
		CAMERA_TYPE	type;
		atik_camera[i]->getCapabilities(&name, &type, &capa);

		switch (device) {
		case DeviceName::Guideport:
			if (capa.hasGuidePort) {
				names.push_back(guideportname(atik_camera[i]));
			}
			break;
		case DeviceName::Camera:
			names.push_back(cameraname(atik_camera[i]));
			break;
		case DeviceName::Ccd:
			names.push_back(ccdname(atik_camera[i], "Imaging"));
			if (capa.has8BitMode) {
				names.push_back(ccdname(atik_camera[i], "8bit"));
			}
			break;
		case DeviceName::Cooler:
			if (capa.cooler != COOLER_NONE) {
				names.push_back(coolername(atik_camera[i]));
			}
			break;
		case DeviceName::Filterwheel:
			if (capa.hasFilterWheel) {
				names.push_back(filterwheelname(atik_camera[i]));
			}
			break;
		default:
			break;
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
 * \param name	name of the camera
 */
CameraPtr	AtikCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve camera %s: converting unit name '%s'",
		sname.c_str(), name.unitname().c_str());
	unsigned int	serial = std::stoul(name.unitname());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	for (int i = 0; i < atik_camera_count; i++) {
		if (atik_camera[i]->getSerialNumber() == serial) {
			return CameraPtr(new AtikCamera(atik_camera[i]));
		}
	}
	std::string	msg = stringprintf("ATIK camera %s not found",
		name.toString().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotFound(msg);
}

} // namespace atik
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	if (NULL == astro::module::atik::atik_locator) {
		astro::module::atik::atik_locator
			= new astro::camera::atik::AtikCameraLocator();
	}
	return astro::module::atik::atik_locator;
}
