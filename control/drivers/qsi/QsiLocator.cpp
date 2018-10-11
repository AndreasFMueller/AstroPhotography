/*
 * QsiLocator.cpp -- camera locator class for QSI cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiLocator.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <includes.h>
#include <qsiapi.h>
#include <QSIError.h>
#include <QsiCamera.h>

namespace astro {
namespace module {
namespace qsi {

//////////////////////////////////////////////////////////////////////
// Implementation of the QSI Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      qsi_name("qsi");
static std::string      qsi_version(VERSION);

/**
 * \brief Module descriptor for the Quantum Scientific Imaging (QSI) module
 */
class QsiDescriptor : public ModuleDescriptor {
public:
	QsiDescriptor() { }
	~QsiDescriptor() { }
        virtual std::string     name() const {
                return qsi_name;
        }
        virtual std::string     version() const {
                return qsi_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace qsi
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::qsi::QsiDescriptor();
}

namespace astro {
namespace camera {
namespace qsi {

//////////////////////////////////////////////////////////////////////
// Implementation of the Camera Locator for QSI
//////////////////////////////////////////////////////////////////////

QsiCameraLocator::QsiCameraLocator() {
}

QsiCameraLocator::~QsiCameraLocator() {
}

/**
 * \brief Get module name.
 */
std::string	QsiCameraLocator::getName() const {
	return std::string("qsi");
}

/**
 * \brief Get module version.
 */
std::string	QsiCameraLocator::getVersion() const {
	QSICamera cam;
	cam.put_UseStructuredExceptions(true);
	try {
		std::string	info;
		cam.get_DriverInfo(info);
		return astro::module::qsi::qsi_version + "/" + info;
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get QSI library version");
		return astro::module::qsi::qsi_version;
	}
}

class unsupported : std::exception {
public:
	unsupported() { }
};

/**
 * \brief Create name of a given type for the camera
 */
std::string	QsiCameraLocator::name(const std::string& serial,
			DeviceName::device_type device) {
	switch (device) {
	case DeviceName::Camera:
		return std::string("camera:qsi/" + serial);
		break;
	case DeviceName::Ccd:
		return std::string("ccd:qsi/" + serial + "/ccd");
		break;
	case DeviceName::Cooler:
		return std::string("cooler:qsi/" + serial + "/ccd/cooler");
		break;
	case DeviceName::Filterwheel:
		return std::string("filterwheel:qsi/" + serial + "/filterwheel");
		break;
	case DeviceName::Guideport:
		return std::string("guideport:qsi/" + serial + "/guideport");
		break;
	default:
		break;
	}
	throw unsupported();
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	QsiCameraLocator::getDevicelist(DeviceName::device_type device) {
	// list of names to return
	std::vector<std::string>	names;

	// exit for all device types not supported by the module
	switch (device) {
	case DeviceName::Camera:
	case DeviceName::Ccd:
	case DeviceName::Cooler:
	case DeviceName::Filterwheel:
	case DeviceName::Guideport:
		break;
	default:
		return names;
	}

	// now get all cameras
	QSICamera cam;
	cam.put_UseStructuredExceptions(true);
	try {
                //Discover the connected cameras
                std::string camSerial[QSICamera::MAXCAMERAS];
                std::string camDesc[QSICamera::MAXCAMERAS];
		int	iNumFound;
                cam.get_AvailableCameras(camSerial, camDesc, iNumFound);
		for (int i = 0; i < iNumFound; i++) {
			std::string	serial = camSerial[i];
			try {
				std::string	devname = name(serial, device);
				names.push_back(devname);
			} catch (unsupported) {
				// skip this device
			}
		}
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during camera retrieval");
	}

	return names;
}

/**
 * \brief static mutex to protect the locator
 *
 * This ensures that only one device can be under construction at any
 * one time. This solves the following problem: When a filterwheel object
 * is constructed, then the camera has to be constructed first, and similarly
 * for other devices. So if Ccd, Cooler and Filterwheel construction are
 * initiated at the same time, the all trigger construction of the
 * same camera. This will make the camera unusable.
 */
static std::recursive_mutex	_mutex;

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	QsiCameraLocator::getCamera0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locating camera %s",
		name.toString().c_str());
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	return CameraPtr(new QsiCamera(name));
}

/**
 * \brief Get a CCD by name
 *
 * \param name	Name of the CCD
 * \return	CcdPtr object of the CCD
 */
CcdPtr	QsiCameraLocator::getCcd0(const DeviceName& ccdname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locating ccd %s",
		ccdname.toString().c_str());
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (ccdname.size() < 2) {
		std::string	msg = stringprintf("bad name: %s",
			ccdname.toString().c_str());
		throw std::runtime_error(msg);
	}
	if (ccdname.unitname() != "ccd") {
		std::string	msg = stringprintf("not a valid unit name: %s",
			ccdname.toString().c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = ccdname.parent(DeviceName::Camera);
	return getCamera(cameraname)->getCcd(0);
}

/**
 * \brief Get a Cooler by name
 *
 * A cooler is retrieved via the CCD
 * \param name	Name of the cooler
 * \return	CoolerPtr object of the Cooler
 */
CoolerPtr	QsiCameraLocator::getCooler0(const DeviceName& coolername) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locating cooler %s",
		coolername.toString().c_str());
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (coolername.size() < 2) {
		std::string	msg = stringprintf("bad name: %s",
			coolername.toString().c_str());
		throw std::runtime_error(msg);
	}
	if (coolername.unitname() != "cooler") {
		std::string	msg = stringprintf("not a valid unit name: %s",
			coolername.toString().c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	ccdname = coolername.parent(DeviceName::Ccd);
	return getCcd(ccdname)->getCooler();
}

/**
 * \brief Get the filterwheel by name
 *
 * \param name	Name of the filterhweel
 * \return	FilterWheelPtr wrapper of the filterwheel of the camera
 */
FilterWheelPtr	QsiCameraLocator::getFilterWheel0(const DeviceName& filterwheelname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locating filterwheel %s",
			filterwheelname.toString().c_str());
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (filterwheelname.size() < 2) {
		std::string	msg = stringprintf("bad name: %s",
			filterwheelname.toString().c_str());
		throw std::runtime_error(msg);
	}
	if (filterwheelname.unitname() != "filterwheel") {
		std::string	msg = stringprintf("not a valid unit name: %s",
			filterwheelname.toString().c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = filterwheelname.parent(DeviceName::Camera);
	return getCamera(cameraname)->getFilterWheel();
}

/**
 * \brief Get the guider port by name
 *
 * \param name	Name of the guider port
 * \return	GuidePortPtr object pointing to the guider port of the camera
 */
GuidePortPtr	QsiCameraLocator::getGuidePort0(const DeviceName& guideportname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locating guideport %s",
		guideportname.toString().c_str());
	std::lock_guard<std::recursive_mutex>	lock(_mutex);
	if (guideportname.size() < 2) {
		std::string	msg = stringprintf("bad name: %s",
			guideportname.toString().c_str());
		throw std::runtime_error(msg);
	}
	if (guideportname.unitname() != "guideport") {
		std::string	msg = stringprintf("not a valid unit name: %s",
			guideportname.toString().c_str());
		throw std::runtime_error(msg);
	}
	DeviceName	cameraname = guideportname.parent(DeviceName::Camera);
	return getCamera(cameraname)->getGuidePort();
}

} // namespace qsi
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::qsi::QsiCameraLocator();
}
