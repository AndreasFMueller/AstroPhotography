/*
 * Qhy2Locator.cpp -- camera locator class for QHYCCD cameras
 *
 * (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
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
        debug(LOG_DEBUG, DEBUG_LOG, 0, "Qhy2Descriptor: %p",
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
std::map<std::string, qhyccd_handle*>	Qhy2CameraLocator::_camera_handles;

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
		auto	rc = InitQHYCCDResource();
		if (rc != QHYCCD_SUCCESS) {
			throw Qhy2Error("InitQHYCCDResource failed", rc);
		}
		initialize_counter++;
	}

#if 0
	// according to QHYCCD, this part of the api no longer needs to
	// be called, it has been integrated in the rest of the API
#ifdef __APPLE__
	// make sure firmware is initialized on OSX
	{
		auto	rc = OSXInitQHYCCDFirmwareArray();
		if (QHYCCD_ERROR != rc) {
			throw Qhy2Error("cannot load firmware", rc);
		}
	}
#endif /* __APPLE__ */
#endif

	// make sure we enumerate the devices or the search functions
	// will fail to find them
	int	camCount = ScanQHYCCD();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d devices found", camCount);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handleForName(%s)", qhyname.c_str());

	// first try to find the handle
	auto	i = _camera_handles.find(qhyname);
	if (i != _camera_handles.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s found in cache",
			qhyname.c_str());
		return i->second;
	}

	// open a new handle
	qhyccd_handle	*handle
		= OpenQHYCCD(const_cast<char *>(qhyname.c_str()));
	if (NULL == handle) {
		std::string	msg = stringprintf("'%s' not found",
			qhyname.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Qhy2Error(msg, -1);
	}

	// initialize the handle
	auto	rc = InitQHYCCD(handle);
	if (rc != QHYCCD_SUCCESS) {
		std::string	msg = stringprintf("cannot initialize '%s': %d",
			qhyname.c_str(), rc);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw Qhy2Error(msg, rc);
	}

	// store the handle
	_camera_handles.insert(std::make_pair(qhyname, handle));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "handle %p cached", handle);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d qhy2 cameras", camCount);
	for (int i = 0; i < camCount; i++) {
		// try to get the camera name
		char	camId[32];
		if (QHYCCD_SUCCESS != GetQHYCCDId(i, camId)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d not a QHYCCD", i);
			continue;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "length: %d", strlen(camId));

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
			if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(handle,
				CONTROL_COOLER)) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"cooler present");
				names.push_back(qhyname.coolername());
			}
			break;
		case DeviceName::Guideport:
			if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(handle,
				CONTROL_ST4PORT)) {
				names.push_back(qhyname.guideportname());
			}
			break;
		case DeviceName::Filterwheel:
			if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(handle,
				CONTROL_CFWPORT)) {
				names.push_back(qhyname.filterwheelname());
			}
			break;
		case DeviceName::Ccd:
			{
				DeviceName	cameraname(qhyname,
							DeviceName::Ccd);
				std::list<DeviceName>	cl = ccdlist(handle,
					cameraname);
				for (auto ccd : cl) {
					names.push_back(ccd);
				}
			}
			break;
		default:
			// no such device known
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting camera %s",
		name.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camid = %s", name[1].c_str());
	Qhy2Camera	*camera = new Qhy2Camera(name[1]);
	CameraPtr	cameraptr(camera);
	return cameraptr;
}

/**
 * \brief Get a cooler from the camera
 *
 * \param name	devicename for a cooler
 */
CoolerPtr	Qhy2CameraLocator::getCooler0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY2 cooler named: %s",
		name.toString().c_str());
	DeviceName	cameraname(name, DeviceName::Camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get camera named %s",
		cameraname.toString().c_str());
	CameraPtr	camera = this->getCamera(cameraname);
	return camera->getCcd(0)->getCooler();
}

/**
 * \brief Get a CCD device for a camera
 */
CcdPtr	Qhy2CameraLocator::getCcd0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY2 ccd named: %s",
		name.toString().c_str());
	DeviceName	cameraname(name, DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);
	return camera->getCcd(name);
}

/**
 * \brief Get a guider port by name
 */
GuidePortPtr	Qhy2CameraLocator::getGuidePort0(const DeviceName& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get QHY2 guideport named: %s",
		name.toString().c_str());
	DeviceName	cameraname(name, DeviceName::Camera);
	CameraPtr	camera = this->getCamera(cameraname);
	return camera->getGuidePort();
}

std::string	canonicalname(char *buffer) {
	char	*p = buffer;
	while (*p) {
		if ((*p == ' ') || (*p == '/')) {
			*p = '_';
		}
		p++;
	}
	return std::string(buffer);
}

/**
 * \brief Retrieve a list of readout modes from a camera identified by handle
 *
 * \param handle	the QHYCCD camera handle
 */
std::vector<std::string>	Qhy2CameraLocator::readmodelist(
					qhyccd_handle *handle) {
	std::vector<std::string>	modes;
	// read the readout modes and names
	uint32_t	number_of_modes = 0;
	int	rc = GetQHYCCDNumberOfReadModes(handle, &number_of_modes);
	if (QHYCCD_SUCCESS == rc) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "number of readout modes: %u",
			number_of_modes);
		for (uint32_t mode = 0; mode < number_of_modes; mode++) {
			char	buffer[1024];
			GetQHYCCDReadModeName(handle, mode, buffer);
			std::string	mode_name = canonicalname(buffer);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new mode '%s' found",
				mode_name.c_str());
			modes.push_back(mode_name);
		}
	} else {
		number_of_modes = 1;
		modes.push_back(std::string("STD"));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d modes", modes.size());
	return modes;
}

/**
 * \brief Retrieve a list of bit depths available for the camera
 *
 * \param handle	the QHYCCD camera handle
 */
std::vector<int>	Qhy2CameraLocator::bitlist(qhyccd_handle *handle) {
	std::vector<int>	bits;
	if (QHYCCD_SUCCESS == IsQHYCCDControlAvailable(handle,
		CONTROL_TRANSFERBIT)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"creating ccds with different bit depths");
		double	min, max, step;
		int	rc = GetQHYCCDParamMinMaxStep(handle,
				CONTROL_TRANSFERBIT, &min, &max, &step);
		if (rc != QHYCCD_SUCCESS) {
			std::string	msg = stringprintf("cannot get "
				"transfer range");
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw Qhy2Error(msg, rc);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"bits from %.0f to %.0f in %.0f steps", min, max, step);
		for (int bit = min; bit <= max + 0.1; bit += step) {
			bits.push_back(bit);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d bit depths", bits.size());
	return bits;
}

/**
 * \brief Build a list of CCD device names
 *
 * \param handle	the QHYCCD camera handle
 * \param camera	the name of the camera
 */
std::list<DeviceName>	Qhy2CameraLocator::ccdlist(qhyccd_handle *handle,
		const DeviceName& camera) {
	std::list<DeviceName>	result;
	std::vector<std::string>	modes = readmodelist(handle);
	std::vector<int>	bits = bitlist(handle);
	for (auto mode : modes) {
		if (bits.size() > 0) {
			for (auto bit : bits) {
				std::string	bitname
					= stringprintf("%d", bit);
				DeviceName	ccdname
					= camera.child(DeviceName::Ccd, mode)
					    .child(DeviceName::Ccd, bitname);
				result.push_back(ccdname);
			}
		} else {
			DeviceName	ccdname = camera.child(DeviceName::Ccd,
						std::string("STD"));
			result.push_back(ccdname);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d ccds", result.size());
	return result;
}

} // namespace qhy2
} // namespace camera
} // namespace astro


extern "C"
astro::device::DeviceLocator    *getDeviceLocator() {
	return new astro::camera::qhy2::Qhy2CameraLocator();
}
