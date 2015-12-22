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

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	QhyCameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;

	// list all devices from the context
	std::vector<usb::DevicePtr>	d = context.devices();
	std::vector<usb::DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		// try to open all devices, and check whether they have
		// the right vendor id
		try {
			usb::DevicePtr	devptr = *i;
			devptr->open();
			try {
				std::string	n = qhyname(devptr);
				if (device == DeviceName::Camera) {
					names.push_back("camera:qhy/" + n);
				}
				if (device == DeviceName::Ccd) {
					names.push_back("ccd:qhy/" + n
						+ "/Imaging");
				}
				if (device == DeviceName::Cooler) {
					names.push_back("cooler:qhy/" + n
						+ "/Imaging/cooler");
				}
				if (device == DeviceName::Guiderport) {
					names.push_back("guiderport:qhy/" + n
						+ "/guiderport");
				}
			} catch (std::runtime_error& x) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "found a non "
					"QHY device: %s", x.what());
			}
		} catch (std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot work with device");
		}
	}

	// return empty list, QSI only has camera devices from the locator
	if (DeviceName::Camera != device) {
		return names;
	}

	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	QhyCameraLocator::getCamera0(const DeviceName& name) {
	std::string	qname = name.unitname();
	int	busnumber = 0, deviceaddress = 0;
	qhyparse(qname, busnumber, deviceaddress);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"constructing camera from %s with bus=%d, addr=%d",
		name.toString().c_str(), busnumber, deviceaddress);

	// scan the devices and get 
	std::vector<usb::DevicePtr>	d = context.devices();
	std::vector<usb::DevicePtr>::const_iterator	i;
	for (i = d.begin(); i != d.end(); i++) {
		usb::DevicePtr	dptr = *i;
		if ((dptr->getBusNumber() == busnumber) &&
			(dptr->getDeviceAddress() == deviceaddress)) {
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
 */
CoolerPtr	QhyCameraLocator::getCooler0(const DeviceName& name) {
	DeviceName	cameraname(DeviceName::Camera, name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for guider port of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	CcdPtr	ccd = camera->getCcd(0);
	if (!ccd->hasCooler()) {
		debug(LOG_ERR, DEBUG_LOG, 0, "camera has no cooler");
		throw NotFound("camera does not have a cooler");
	}
	return ccd->getCooler();
}

/**
 * \brief Get a CCD device for a camera
 */
CcdPtr	QhyCameraLocator::getCcd0(const DeviceName& name) {
	DeviceName	cameraname(DeviceName::Camera, name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for guider port of camera %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	return camera->getCcd(0);
}

/**
 * \brief Get a guider port by name
 */
GuiderPortPtr	QhyCameraLocator::getGuiderPort0(const DeviceName& name) {
	DeviceName	cameraname(DeviceName::Camera, name);
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
