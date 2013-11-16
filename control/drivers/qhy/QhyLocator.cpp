/*
 * QhyLocator.cpp -- camera locator class for QSI cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyLocator.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroLoader.h>
#include <includes.h>

namespace astro {
namespace module {
namespace qhy {

//////////////////////////////////////////////////////////////////////
// Implementation of the QSI Module Descriptor
//////////////////////////////////////////////////////////////////////

static std::string      qhy_name("qhy");
static std::string      qhy_version(VERSION);

/**
 * \brief Module descriptor for the Starlight express module
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
	return CameraPtr();
	//return CameraPtr(new QhyCamera(name));
}

} // namespace qhy
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::qhy::QhyCameraLocator();
}
