/*
 * Qhy2Locator.cpp -- camera locator class for QSI cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Locator.h>
#include <Qhy2Utils.h>
#include <Qhy2Camera.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <includes.h>
#include <qhyccd.h>

namespace astro {
namespace module {
namespace qhy2 {

#define QHY_VENDOR_ID	0x1618

//////////////////////////////////////////////////////////////////////
// Implementation of the QSI Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      qhy_name("qhy2");
static std::string      qhy_version(VERSION);

/**
 * \brief Module descriptor for the QHY module
 */
class Qhy2Descriptor : public ModuleDescriptor {
public:
	Qhy2Descriptor() { }
	~Qhy2Descriptor() { }
        virtual std::string     name() const {
                return qhy_name;
        }
        virtual std::string     version() const {
                return qhy_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

static std::once_flag   descriptor_once;
static astro::module::ModuleDescriptor  *descriptor;
void	setup_descriptor() {
	descriptor = new Qhy2Descriptor();
}

} // namespace qhy2
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
        std::call_once(astro::module::qhy2::descriptor_once,
                astro::module::qhy2::setup_descriptor);
        debug(LOG_DEBUG, DEBUG_LOG, 0, "QsiDescriptor: %p",
                astro::module::qhy2::descriptor);
        return astro::module::qhy2::descriptor;
}

namespace astro {
namespace camera {
namespace qhy2 {

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for QSI
//////////////////////////////////////////////////////////////////////

Qhy2CameraLocator::Qhy2CameraLocator() {
	// context.setDebugLevel(0);
}

Qhy2CameraLocator::~Qhy2CameraLocator() {
}

/**
 * \brief Get module name.
 */
std::string	Qhy2CameraLocator::getName() const {
	return std::string("qhy2");
}

/**
 * \brief Get module version.
 */
std::string	Qhy2CameraLocator::getVersion() const {
	return astro::module::qhy2::qhy_version;
}

static void	addname(std::vector<std::string>& names, usb::DevicePtr devptr,
	DeviceName::device_type device) {
#if 0
	Qhy2Name	qhyname(devptr);
	switch (device) {
	case DeviceName::Camera:
		names.push_back(qhyname.cameraname());
		break;
	case DeviceName::Ccd:
		names.push_back(qhyname.ccdname());
		break;
	case DeviceName::Cooler:
		names.push_back(qhyname.coolername());
		break;
	case DeviceName::Guideport:
		names.push_back(qhyname.guideportname());
		break;
	default:
		// unknown components
		break;
	}
#endif
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \param device	the type of devices we want to have listed
 * \return 		a vector of strings that uniquely descript devices
 */
std::vector<std::string>	Qhy2CameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;

#if 0
	// list all devices from the context
	std::vector<usb::DevicePtr>	d = context.devices();
	std::vector<usb::DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		usb::DevicePtr	devptr = *i;
		// try to open all devices, and check whether they have
		// the right vendor id
		try {
			devptr->open();
			try {
				addname(names, devptr, device);
			} catch (std::runtime_error& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "found a non "
					"QHY device: %s", x.what());
			}
		} catch (std::exception& x) {
			std::string	msg = stringprintf("cannot work with "
				"device at bus=%d and addr=%d",
				devptr->getBusNumber(),
				devptr->getDeviceAddress());
			debug(LOG_ERR, DEBUG_LOG, 0, msg.c_str());
		}
	}
#endif

	// return the list of devices
	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return 		Camera with that name
 */
CameraPtr	Qhy2CameraLocator::getCamera0(const DeviceName& name) {
#if 0
	Qhy2Name	qhyname(name);
	if (!qhyname.isCamera(name)) {
		std::string	msg = stringprintf("%s is not a Camera name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// scan the devices and get 
	std::vector<usb::DevicePtr>	d = context.devices();
	std::vector<usb::DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		usb::DevicePtr	dptr = *i;
		int	busnumber = dptr->getBusNumber();
		int	deviceaddress = dptr->getDeviceAddress();
		if ((busnumber == qhyname.busnumber()) &&
			(deviceaddress == qhyname.deviceaddress())) {
			dptr->open();
			return CameraPtr(new Qhy2Camera(dptr));
		}
	}

	// failure to construct the camera
	std::string	msg = stringprintf("cannot create camera from '%s'",
		name.toString().c_str());
	throw std::runtime_error(msg);
#else
	CameraPtr	camera;
	return camera;
#endif
}

/**
 * \brief Get a cooler from the camera
 *
 * \param name	devicename for a cooler
 */
CoolerPtr	Qhy2CameraLocator::getCooler0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY cooler named: %s",
		name.toString().c_str());
#if 0
	Qhy2Name	qhyname(name);
	if (!qhyname.isCooler(name)) {
		std::string	msg = stringprintf("%s is not a Cooler name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = qhyname.cameraname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for cooler of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	CcdPtr	ccd = camera->getCcd(0);
	if (!ccd->hasCooler()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera has no cooler");
		throw NotFound("camera does not have a cooler");
	}
	CoolerPtr	result = ccd->getCooler();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got cooler named '%s'",
		result->name().toString().c_str());
	return result;
#else
	CoolerPtr	cooler;
	return cooler;
#endif
}

/**
 * \brief Get a CCD device for a camera
 */
CcdPtr	Qhy2CameraLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY ccd named: %s",
		name.toString().c_str());
#if 0
	Qhy2Name	qhyname(name);
	if (!qhyname.isCcd(name)) {
		std::string	msg = stringprintf("%s is not a CCD name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	DeviceName	cameraname = qhyname.cameraname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for CCD of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	return camera->getCcd(0);
#else
	CcdPtr	ccd;
	return ccd;
#endif
}

/**
 * \brief Get a guider port by name
 */
GuidePortPtr	Qhy2CameraLocator::getGuidePort0(const DeviceName& name) {
#if 0
	Qhy2Name	qhyname(name);
	if (!qhyname.isGuideport(name)) {
		std::string	msg = stringprintf("%s is not a Guideport name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = qhyname.cameraname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for guider port of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	if (!camera->hasGuidePort()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera has no guider port");
		throw NotFound("camera does not have a guider port");
	}
	return GuidePortPtr();
#else
	GuidePortPtr	guideport;
	return guideport;
#endif
}

} // namespace qhy2
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::qhy2::Qhy2CameraLocator();
}
