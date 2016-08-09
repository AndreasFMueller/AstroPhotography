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
		atik_camera[i]->open();
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
	switch (device) {
	case DeviceName::AdaptiveOptics:
	case DeviceName::Focuser:
	case DeviceName::Module:
	case DeviceName::Mount:
		return names;
	case DeviceName::Filterwheel:
		return names;
	case DeviceName::Guiderport:
		return names;
	case DeviceName::Camera:
		for (int i = 0; i < atik_camera_count; i++) {
			names.push_back(cameraname(atik_camera[i]));
		}
		return names;
	case DeviceName::Ccd:
		return names;
	case DeviceName::Cooler:
		for (int i = 0; i < atik_camera_count; i++) {
			
		}
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
 * \param name	name of the camera
 */
CameraPtr	AtikCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	unsigned int	serial = std::stoi(name.unitname());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	for (int i = 0; i < atik_camera_count; i++) {
		if (atik_camera[i]->getSerialNumber() == serial) {
			// XXX create a new camera
		}
	}
	throw std::runtime_error("not implemented yet");
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
