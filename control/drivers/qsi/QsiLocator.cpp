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

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \return a vector of strings that uniquely descript devices
 */
std::vector<std::string>	QsiCameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;

	// return empty list, QSI only has camera devices from the locator
	if (DeviceName::Camera != device) {
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
			std::string	cameraname =
				std::string("camera:qsi/") + camSerial[i];
			names.push_back(cameraname);
		}
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during camera retrieval");
	}

	return names;
}

/**
 * \brief Construct a camera from a camera description
 *
 * \param name		Name of the camera
 * \return Camera with that name
 */
CameraPtr	QsiCameraLocator::getCamera0(const DeviceName& name) {
	return CameraPtr(new QsiCamera(name));
}

} // namespace qsi
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::qsi::QsiCameraLocator();
}
