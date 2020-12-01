/*
 * Qhy2Locator.cpp -- camera locator class for QHYCCD cameras
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
#include <atomic>

namespace astro {
namespace module {
namespace qhy2 {

#define QHY_VENDOR_ID	0x1618

//////////////////////////////////////////////////////////////////////
// Implementation of the QHYCCD Module Descriptor
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
// Implementation of the Camera Locator for QHYCCD
//////////////////////////////////////////////////////////////////////

static int	initialize_counter;
static std::recursive_mutex	initialize_mutex;
static std::once_flag	initialize_once;

static void	initialize() {
	initialize_counter = 0;
}

/**
 * \brief Constructor for the QhyLocator
 *
 * This constructor is responsible for initializing the QHYCCD resources
 * through the InitQHYCCDResource call. Each time the constructor is
 * called, the initialize_counter is increased, the InitQHYCCDResource
 * call is only needed when the counter is zero.
 */
Qhy2CameraLocator::Qhy2CameraLocator() {
	std::call_once(initialize_once, initialize);
	std::unique_lock<std::recursive_mutex>	lock(initialize_mutex);
	if (initialize_counter == 0) {
		int	rc = InitQHYCCDResource();
		if (rc != QHYCCD_SUCCESS) {
			throw Qhy2Error("InitQHYCCDResource failed", rc);
		}
		initialize_counter++;
	}
}

/**
 * \brief Destructor for the Qhy2 locator
 *
 * This destructor decrements initialize_counter each time it is called.
 * as soon as it reaches zero, it calls ReleaseQHYCCDResource(). 
 */
Qhy2CameraLocator::~Qhy2CameraLocator() {
	std::unique_lock<std::recursive_mutex>	lock(initialize_mutex);
	initialize_counter--;
	if (0 == initialize_counter) {
		int	rc = ReleaseQHYCCDResource();
		if (rc != QHYCCD_SUCCESS) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"ReleaseQHYCCDResource() failed %d (ignored)",
				rc);
		}
	}
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

/**
 * \brief Retreive the handle for this camera
 *
 * \param qhyname	the camera name used in the QHYCCD api
 */
qhyccd_handle	*Qhy2CameraLocator::handleForName(const std::string& qhyname) {
	auto	i = _camera_handles.find(qhyname);
	if (i != _camera_handles.end()) {
		return i->second;
	}
	qhyccd_handle	*handle = OpenQHYCCD(const_cast<char *>(qhyname.c_str()));
	if (NULL == handle) {
		std::string	msg = stringprintf("'%s' not found",
			qhyname.c_str());
		throw Qhy2Error(msg, -1);
	}
	_camera_handles.insert(std::make_pair(qhyname, handle));
	return handle;
}

/**
 * \brief Retrieve the camera handle for this device 
 *
 * \param devicename	the device name of the device
 */
qhyccd_handle	*Qhy2CameraLocator::handleForName(const DeviceName& devicename) {
	return handleForName(devicename[1]);
}

/**
 * \brief Get a list of Starlight Express cameras.
 *
 * \param device	the type of devices we want to have listed
 * \return 		a vector of strings that uniquely descript devices
 */
std::vector<std::string>	Qhy2CameraLocator::getDevicelist(DeviceName::device_type device) {
	std::vector<std::string>	names;

	// scan for cameras
	int	camCount = ScanQHYCCD();
	for (int i = 0; i < camCount; i++) {
		// try to get the camera name
		char	camId[32];
		if (QHYCCD_SUCCESS != GetQHYCCDId(i, camId)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d not a QHYCCD", i);
			continue;
		}

		// use the camera name and id to build the name
		Qhy2Name	qhyname(i);
		if (device == DeviceName::Camera) {
			// add the camera name
			names.push_back(qhyname);
			continue;
		}

		// we have to further investigate whether the camera has
		// a cooler or a filter wheel
		qhyccd_handle	*handle = handleForName(qhyname);
		switch (device) {
		case DeviceName::Cooler:
			if (IsQHYCCDControlAvailable(handle, CONTROL_COOLER)) {
				names.push_back(qhyname.coolername());
			}
			break;
		case DeviceName::Guideport:
			if (IsQHYCCDControlAvailable(handle, CONTROL_ST4PORT)) {
				names.push_back(qhyname.guideportname());
			}
			break;
		case DeviceName::Filterwheel:
			if (IsQHYCCDControlAvailable(handle, CONTROL_CFWPORT)) {
				names.push_back(qhyname.filterwheelname());
			}
			break;
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
