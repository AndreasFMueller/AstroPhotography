/*
 * SbigLocator.cpp -- camera locator for SBIG driver cameras
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_LPARDRV_H
#include <lpardrv.h>
#else
#ifdef HAVE_SBIGUDRV_LPARDRV_H
#include <SBIGUDrv/lpardrv.h>
#endif /* HAVE_SBIGUDRV_LPARDRV_H */
#endif

#include <SbigLock.h>
#include <SbigLocator.h>
#include <AstroDebug.h>
#include <utils.h>
#include <AstroFormat.h>
#include <SbigCamera.h>
#include <includes.h>
#include <SbigFilterWheel.h>

//////////////////////////////////////////////////////////////////////
// Implementation of the SBIG Express Module Descriptor
//////////////////////////////////////////////////////////////////////

namespace astro {
namespace module {
namespace sbig {

static std::string      sbig_name("sbig");
static std::string      sbig_version(VERSION);
static astro::camera::sbig::SbigCameraLocator	*sbig_locator = NULL;

/**
 * \brief Module descriptor for the SBIG module
 */
class SbigDescriptor : public ModuleDescriptor {
public:
	SbigDescriptor() { }
	~SbigDescriptor() { }
        virtual std::string     name() const {
                return sbig_name;
        }
        virtual std::string     version() const {
                return sbig_version;
        }
	virtual bool	hasDeviceLocator() const {
		return true;
	}
};

} // namespace sbig
} // namespace module
} // namespace astro

extern "C"
astro::module::ModuleDescriptor	*getDescriptor() {
	return new astro::module::sbig::SbigDescriptor();
}

using namespace astro;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

//////////////////////////////////////////////////////////////////////
// SbigLocator implementation
//////////////////////////////////////////////////////////////////////


int	SbigCameraLocator::driveropen = 0;

std::string	SbigCameraLocator::getName() const {
	return std::string("sbig");
}

std::string	SbigCameraLocator::getVersion() const {
	return VERSION;
}

/**
 * \brief Constructor for the SbigCameraLocator class
 *
 * Constructor and Destructor keep track of how many locators have been
 * created because the the last locator destroyed has to also close
 * the driver.
 */
SbigCameraLocator::SbigCameraLocator() {
	if (0 == driveropen) {
		short	e = SBIGUnivDrvCommand(CC_OPEN_DRIVER, NULL, NULL);
		if (e != CE_NO_ERROR) {
			std::string	errmsg = sbig_error(e);
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot open driver: %s",
				errmsg.c_str());
			throw SbigError(errmsg.c_str());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driver opened: %hd", e);
		getNames();
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driver already open");
	}
	driveropen++;
}

/**
 * \brief Destructor for the locator
 *
 * The destructor counts down the driveropen static variable
 */
SbigCameraLocator::~SbigCameraLocator() {
	if (0 == --driveropen) {
		clearNames();
		short	e = SBIGUnivDrvCommand(CC_CLOSE_DRIVER, NULL, NULL);
		if (e != CE_NO_ERROR) {
			std::string	errmsg = sbig_error(e);
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot close driver: %s",
				errmsg.c_str());
			throw SbigError(errmsg.c_str());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "driver closed: %hd", e);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d remaining driver references",
			driveropen);
	}
}

/**
 * \brief auxiliary function to generate camera name
 *
 * The name of an SBIG camera is essentially the serial number of the camera
 */
static DeviceName	sbigCameraName(const QueryUSBResults& queryresults,
				int index) {
	std::string	name(queryresults.usbInfo[index].serialNumber);
	DeviceName	cameraname(DeviceName::Camera, "sbig", name);
	return cameraname;
}

/**
 * \brief Generate a guider port name from the camera
 *
 * The name generated is designed to work with the implementation of the
 * getGuideport0 function in the base DeviceLocator class, so that no
 * SBIG-specific implementation of this function is required.
 */
static void	sbigAddGuideportName(std::vector<std::string>& names,
				const QueryUSBResults& queryresult,
				int index) {
	DeviceName	cameraname = sbigCameraName(queryresult, index);
	std::string	guideportname = cameraname.child(DeviceName::Guideport,
				"guideport");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding guideport %s",
		guideportname.c_str());
	names.push_back(guideportname);
}

/**
 * \brief Generate a filterwheel name
 *
 * Note that getFilterwheel0 does not have a standard implementation in
 * the DeviceLocator base class, so we will need an SBIG specific
 * implementation anyway.
 */
static void	sbigAddFilterwheelName(std::vector<std::string>& names,
				const QueryUSBResults& queryresult, int index) {
	DeviceName	cameraname = sbigCameraName(queryresult, index);
	DeviceName	filterwheelname(cameraname);
	filterwheelname.type(DeviceName::Filterwheel);

	// XXX temporary hack
	switch (queryresult.usbInfo[index].cameraType) {
	case STX_CAMERA:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding filterwheel %s",
			filterwheelname.toString().c_str());
		names.push_back(filterwheelname);
		break;
	default:
		break;
	}
	return;

	// XXX what we should really do is the following
	// XXX open the device
	// XXX use the CFWC_GET_INFO command to get information about the FW
	CFWParams	params;
	params.cfwModel = CFWSEL_AUTO;
	params.cfwCommand = CFWC_GET_INFO;
	params.cfwParam1 = CFWG_FIRMWARE_VERSION;
	// if this succeeds, then we know that we have a filter wheel,
	// and we add the name
	CFWResults	results;
	short	e = SBIGUnivDrvCommand(CC_CFW, &params, &results);
	if (e == CE_NO_ERROR) {
		// a filterwheel was found
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel found");
		names.push_back(filterwheelname);
	}

	// XXX close the device again
}

static void	sbigAddCcdName(std::vector<std::string>& names,
			const QueryUSBResults& queryresult, int index) {
	DeviceName	cameraname = sbigCameraName(queryresult, index);
	std::string	ccd = cameraname.child(DeviceName::Ccd, "Imaging");
	names.push_back(ccd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding imaging ccd %s", ccd.c_str());

	switch (queryresult.usbInfo[index].cameraType) {
	case ST7_CAMERA:
	case ST8_CAMERA:
	case ST5C_CAMERA:
		break;
	case TCE_CONTROLLER:
		return;
	case ST237_CAMERA:
	case STK_CAMERA:
	case ST9_CAMERA:
	case STV_CAMERA:
	case ST10_CAMERA:
	case ST1K_CAMERA:
	case ST2K_CAMERA:
	case STL_CAMERA:
	case ST402_CAMERA:
	case STX_CAMERA:
	case ST4K_CAMERA:
	case STT_CAMERA:
	case STI_CAMERA:
	case STF_CAMERA:
		break;
	case NEXT_CAMERA:
	case NO_CAMERA:
		return;
	}

	std::string	gccd = cameraname.child(DeviceName::Ccd, "Tracking");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding guiding ccd %s", gccd.c_str());
	names.push_back(gccd);
}

/**
 * \brief Generate name for cooler
 *
 * Generate a cooler name that works with the default implementation of
 * the getCooler0 method in the DeviceLocator base class.
 */
static void	sbigAddCoolerName(std::vector<std::string>& names,
			const QueryUSBResults& queryresult,
			int index) {
	DeviceName	cameraname = sbigCameraName(queryresult, index);
	switch (queryresult.usbInfo[index].cameraType) {
	case ST7_CAMERA:
	case ST8_CAMERA:
	case ST5C_CAMERA:
	case TCE_CONTROLLER:
	case ST9_CAMERA:
	case ST10_CAMERA:
	case ST1K_CAMERA:
	case ST2K_CAMERA:
	case STL_CAMERA:
	case ST402_CAMERA:
	case STX_CAMERA:
	case ST4K_CAMERA:
	case STT_CAMERA:
	case STF_CAMERA:
		break;
	default:
		return;
		break;
	}
	std::string	cooler
		= cameraname.child(DeviceName::Ccd,"Imaging")
			.child(DeviceName::Cooler, "cooler");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding cooler %s", cooler.c_str());
	names.push_back(cooler);
}

/**
 * \brief Get a list of SBIG cameras
 *
 * The cameras on the USB bus are numbers, that's the order in which the
 * locator returns the identifying string of the camera. A camera is
 * identified by its serial number an name.
 */
std::vector<std::string>	SbigCameraLocator::getDevicelist(DeviceName::device_type device) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get SBIG device list for %s",
		DeviceName::type2string(device).c_str());

	// there are no focusers, adaptive optics units, modules and mounts
	// that we could return, so for these object types we return an
	// empty list
	std::vector<std::string>	names;
	switch (device) {
	case DeviceName::Camera:
		return _cameras;
	case DeviceName::Ccd:
		return _ccds;
	case DeviceName::Cooler:
		return _coolers;
	case DeviceName::Filterwheel:
		return _filterwheels;
	case DeviceName::Guideport:
		return _guideports;
	default:
		return names;
	}
}

/**
 * \brief Method to build the list of device names 
 *
 * The SBIG driver library is not flexible enough to learn about new
 * devices after the library has been opened. In particular, it can
 * not scan for cameras when one of the cameras is still open. To make
 * up for this stupidity, we have to get all the names at the beginning.
 */
void	SbigCameraLocator::getNames() {
	// for all other types of names we consult the 
	QueryUSBResults	results;
	SbigLock	lock;
	short	e = SBIGUnivDrvCommand(CC_QUERY_USB, NULL, &results);
	if (e != CE_NO_ERROR) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get camera list: %s",
			sbig_error(e).c_str());
		throw SbigError(e);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d cameras",
		results.camerasFound);
	for (int i = 0; i < results.camerasFound; i++) {
		if (results.usbInfo[i].cameraFound) {
			std::string	cameraname = sbigCameraName(results, i);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "adding camera %s",
				cameraname.c_str());
				_cameras.push_back(cameraname);
			sbigAddGuideportName(_guideports, results, i);
			sbigAddCcdName(_ccds, results, i);
			sbigAddCoolerName(_coolers, results, i);
			sbigAddFilterwheelName(_filterwheels, results, i);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "names found: %d cameras, %d ccds, "
		"%d coolers, %d filterwheels, %d guideports",
		_cameras.size(), _ccds.size(), _coolers.size(),
		_filterwheels.size(), _guideports.size());
}

/**
 * \brief Clear the list of names
 *
 * This method should only be called when closing the driver library
 */
void	SbigCameraLocator::clearNames() {
	_cameras.clear();
	_ccds.clear();
	_coolers.clear();
	_filterwheels.clear();
	_guideports.clear();
}

std::vector<std::string>	SbigCameraLocator::_cameras;
std::vector<std::string>	SbigCameraLocator::_ccds;
std::vector<std::string>	SbigCameraLocator::_coolers;
std::vector<std::string>	SbigCameraLocator::_filterwheels;
std::vector<std::string>	SbigCameraLocator::_guideports;

/**
 * \brief Get a camera by name
 *
 * This works by retrieving a list of cameras and the checking which number
 * has the right name. This index is then used to retreive the camera object
 * by number.
 */
CameraPtr	SbigCameraLocator::getCamera0(const DeviceName& name) {
	std::string	sname = name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "locate camera %s", sname.c_str());
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	size_t	index = 0;
	for (i = cameras.begin(); i != cameras.end(); i++, index++) {
		if (name == *i) {
			return CameraPtr(new SbigCamera(index));
		}
	}
	std::string	msg = stringprintf("camera %s not found", sname.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Get a filterwheel by name
 *
 * This function retrieves the filterwheel attached to a camera.
 *
 * \param name	name of the filterwheel
 */
FilterWheelPtr	SbigCameraLocator::getFilterWheel0(const DeviceName& name) {
	// get the corresponding camera name
	DeviceName	cameraname = name;
	cameraname.type(DeviceName::Camera);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for camera %s",
		cameraname.toString().c_str());

	// get the list of cameras
	std::vector<std::string>	cameras = getDevicelist();
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		if (cameraname == *i) {
			CameraPtr	camera = getCamera(cameraname);
			SbigCamera	*sbigcam = dynamic_cast<SbigCamera *>(&*camera);
			return FilterWheelPtr(new SbigFilterWheel(*sbigcam));
		}
	}
	std::string	msg = stringprintf("filterhweel %s not found",
				name.toString().c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

} // namespace sbig
} // namespace camera
} // namespace astro

extern "C"
astro::device::DeviceLocator	*getDeviceLocator() {
	if (NULL == astro::module::sbig::sbig_locator) {
		astro::module::sbig::sbig_locator
			= new astro::camera::sbig::SbigCameraLocator();
	}
	return astro::module::sbig::sbig_locator;
}
