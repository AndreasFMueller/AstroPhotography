/*
 * QhyLocator.cpp -- camera locator class for QSI cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyLocator.h>
#include <QhyUtils.h>
#include <QhyCamera.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <includes.h>
#include <qhylib.h>

namespace astro {
namespace module {
namespace qhy {

#define QHY_VENDOR_ID	0x1618

//////////////////////////////////////////////////////////////////////
// Implementation of the QSI Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      qhy_name("qhy");
static std::string      qhy_version(VERSION);

/**
 * \brief Module descriptor for the QHY module
 */
class QhyDescriptor : public ModuleDescriptor {
public:
	QhyDescriptor() { }
	~QhyDescriptor() { }
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

} // namespace qhy
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::qhy::QhyDescriptor();
}

namespace astro {
namespace camera {
namespace qhy {

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for QSI
//////////////////////////////////////////////////////////////////////

QhyCameraLocator::QhyCameraLocator() {
	context.setDebugLevel(3);
}

QhyCameraLocator::~QhyCameraLocator() {
}

/**
 * \brief Get module name.
 */
std::string	QhyCameraLocator::getName() const {
	return std::string("qhy");
}

/**
 * \brief Get module version.
 */
std::string	QhyCameraLocator::getVersion() const {
	return astro::module::qhy::qhy_version;
}

static void	addname(std::vector<std::string>& names, usb::DevicePtr devptr,
	DeviceName::device_type device) {
	QhyName	qhyname(devptr);
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
	case DeviceName::Guiderport:
		names.push_back(qhyname.guiderportname());
		break;
	default:
		// unknown components
		break;
	}
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \param device	the type of devices we want to have listed
 * \return 		a vector of strings that uniquely descript devices
 */
std::vector<std::string>	QhyCameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;

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

	// return the list of devices
	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return 		Camera with that name
 */
CameraPtr	QhyCameraLocator::getCamera0(const DeviceName& name) {
	QhyName	qhyname(name);
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
			return CameraPtr(new QhyCamera(dptr));
		}
	}

	// failure to construct the camera
	std::string	msg = stringprintf("cannot create camera from '%s'",
		name.toString().c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get a cooler from the camera
 *
 * \param name	devicename for a cooler
 */
CoolerPtr	QhyCameraLocator::getCooler0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY cooler named: %s",
		name.toString().c_str());
	QhyName	qhyname(name);
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
}

/**
 * \brief Get a CCD device for a camera
 */
CcdPtr	QhyCameraLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY ccd named: %s",
		name.toString().c_str());
	QhyName	qhyname(name);
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
}

/**
 * \brief Get a guider port by name
 */
GuiderPortPtr	QhyCameraLocator::getGuiderPort0(const DeviceName& name) {
	QhyName	qhyname(name);
	if (!qhyname.isGuiderport(name)) {
		std::string	msg = stringprintf("%s is not a Guiderport name",
			name.toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = qhyname.cameraname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for guider port of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	if (!camera->hasGuiderPort()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera has no guider port");
		throw NotFound("camera does not have a guider port");
	}
	return GuiderPortPtr();
}

} // namespace qhy
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::qhy::QhyCameraLocator();
}
