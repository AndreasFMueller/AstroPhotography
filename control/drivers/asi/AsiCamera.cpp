/*
 * AsiCamera.cpp -- ASI camera implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiCamera.hh>
#include <utils.h>
#include <AsiCcd.h>
#include <AsiGuiderPort.h>
#include <AsiLocator.h>
#include <ASICamera2.h>

namespace astro {
namespace camera {
namespace asi {

/**
 * \brief Construct an AsiCamera
 *
 * \param _index	index of the camera
 */
AsiCamera::AsiCamera(int index) : Camera(asiCameraName(index)),
	_index(index) {
	// if the camera is already open, this constructors should not be
	// called
	if (AsiCameraLocator::isopen(_index)) {
		std::string	msg = stringprintf("%s: internal error, "
			"already open", name().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// open the camera
	int	rc = ASIOpenCamera(index);
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("%s: cannot open",
			name().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	AsiCameraLocator::setopen(_index, true);

	// get information about the CCD
	ASI_CAMERA_INFO camerainfo;
        if (ASI_SUCCESS != (rc = ASIGetCameraProperty(&camerainfo, _index))) {
		std::string	msg = stringprintf("%s: cannot get props: %s",
			name().toString().c_str(), error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// set common variables depending on the camera info
	_hasGuiderPort = (camerainfo.ST4Port) ? true : false;
	_isColor = (camerainfo.IsColorCam) ? true : false;
	_hasCooler = (camerainfo.IsCoolerCam) ? true : false;
	_id = camerainfo.CameraID;
	ImageSize	size(camerainfo.MaxWidth, camerainfo.MaxHeight);

	// construct a CcdInfo object for each image format
	std::vector<std::string>	ccdnames
		= AsiCameraLocator::imgtypes(index);
	std::vector<std::string>::const_iterator	i;
	for (i = ccdnames.begin(); i != ccdnames.end(); i++) {
		// construct the name for this 
		DeviceName	ccdname = name().child(DeviceName::Ccd, *i);
		CcdInfo	info(ccdname, size, 0);

		// pixel size
		info.pixelwidth(camerainfo.PixelSize * 1e-6);
		info.pixelheight(camerainfo.PixelSize * 1e-6);

		// add all binning modes
		int	binindex = 0;
		while (camerainfo.SupportedBins[binindex]) {
			int	bin = camerainfo.SupportedBins[binindex];
			info.addMode(Binning(bin, bin));
			binindex++;
		}

		// ASI cameras have no shutter
		info.shutter(false);

		// add the ccdinfo object to the array
		ccdinfo.push_back(info);
	}
}

/**
 * \brief Destroy the AsiCamera
 */
AsiCamera::~AsiCamera() {
	int	rc;
	if (ASI_SUCCESS != (rc = ASICloseCamera(_id))) {
		std::string	msg = stringprintf("%s cannot close camera: %s",
			name().toString().c_str(), error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	AsiCameraLocator::setopen(_index, false);
}

/**
 * \brief Get a CCD from 
 */
CcdPtr	AsiCamera::getCcd0(size_t id) {
	if (id >= nCcds()) {
		std::string	msg = stringprintf("ccd %d does not exist",
			id);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	AsiCcd	*ccd = new AsiCcd(ccdinfo[id], *this);
	ccd->hasCooler(_hasCooler);
	return CcdPtr(ccd);
}

/**
 * \brief Does the camera have guider port?
 *
 * Always returns true, because all ASI cameras have a guider port
 */
bool    AsiCamera::hasGuiderPort() const {
	return _hasGuiderPort;
}

/**
 * \brief Get a guider port
 */
GuiderPortPtr	AsiCamera::getGuiderPort0() {
	return GuiderPortPtr(new AsiGuiderPort(*this));
}

/**
 * \brief Is this a color camera?
 *
 * The variable _isColor was set during construction of the camera
 */
bool	AsiCamera::isColor() const {
	return _isColor;
}

/**
 * \brief Get the index of a control based on the name
 */
int	AsiCamera::controlIndex(const std::string& controlname) {
	int	n;
	int	rc;
	if (ASI_SUCCESS != (rc = ASIGetNumOfControls(_id, &n))) {
		std::string	msg = stringprintf("%s cannot get controls: %s",
			name().toString().c_str(),
			AsiCamera::error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	for (int i = 0; i < n; i++) {
		ASI_CONTROL_CAPS	caps;
		int	rc;
		if (ASI_SUCCESS != (rc = ASIGetControlCaps(_id, i, &caps))) {
			std::string	msg = stringprintf("%s: cannot get "
				"capability %d: %s", name().toString().c_str(),
				i, AsiCamera::error(rc).c_str());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (controlname == std::string(caps.Name)) {
			return i;
		}
	}
	std::string	msg = stringprintf("%s no control %s",
			name().toString().c_str(), controlname.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Common function to retrieve camera capabilities
 */
static void	getControlCapabilities(int id, int control_index,
	ASI_CONTROL_CAPS *caps) {
	int	rc;
	if (ASI_SUCCESS != (rc = ASIGetControlCaps(id, control_index,
		caps))) {
		std::string	msg = stringprintf("%d cannot get caps: %s",
			id, AsiCamera::error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}

/**
 * \brief get the maximum value of a control by index
 */
long    AsiCamera::controlMax(int control_index) {
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return caps.MaxValue;
}

/**
 * \brief get the maximum value of a control by name
 */
long    AsiCamera::controlMax(const std::string& controlname) {
	return controlMin(controlIndex(controlname));
}

/**
 * \brief get the minimum value of a control by index
 */
long    AsiCamera::controlMin(int control_index) {
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return caps.MinValue;
}

/**
 * \brief get the minimum value of a control by name
 */
long    AsiCamera::controlMin(const std::string& controlname) {
	return controlMin(controlIndex(controlname));
}

/**
 * \brief get the default value of a control by index
 */
long    AsiCamera::controlDefault(int control_index) {
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return caps.DefaultValue;
}

/**
 * \brief get the default value of a control by name
 */
long    AsiCamera::controlDefault(const std::string& controlname) {
	return controlDefault(controlIndex(controlname));
}

/**
 * \brief Get the name of a control by index
 */
std::string     AsiCamera::controlName(int control_index) {
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return std::string(caps.Name);
}

/**
 * \brief Get the name of a control by name
 */
std::string     AsiCamera::controlName(const std::string& controlname) {
	return controlName(controlIndex(controlname));
}

/**
 * \brief Get the description of a control by index
 */
std::string     AsiCamera::controlDescription(int control_index) {
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return std::string(caps.Description);
}

/**
 * \brief Get the description of a control by name
 */
std::string     AsiCamera::controlDescription(const std::string& controlname) {
	return controlDescription(controlIndex(controlname));
}

/**
 * \brief get whether a control is writabl by index
 */
bool    AsiCamera::controlWritable(int control_index) {
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return (caps.IsWritable) ? true : false;
}

/**
 * \brief get whether a control is writabl by name
 */
bool    AsiCamera::controlWritable(const std::string& controlname) {
	return controlWritable(controlIndex(controlname));
}

/**
 * \brief convert from C++ asi control types to the ASI API constants
 */
static ASI_CONTROL_TYPE	type2asitype(AsiControlType type) {
	switch (type) {
	case AsiGain:			return ASI_GAIN;
        case AsiExposure:		return ASI_EXPOSURE;
        case AsiGamma:			return ASI_GAMMA;
        case AsiWbR:			return ASI_WB_R;
        case AsiWbB:			return ASI_WB_B;
        case AsiBrightness:		return ASI_BRIGHTNESS;
        case AsiBandwithoverload:	return ASI_BANDWIDTHOVERLOAD;
        case AsiOverclock:		return ASI_OVERCLOCK;
        case AsiTemperature:		return ASI_TEMPERATURE;
        case AsiFlip:			return ASI_FLIP;
        case AsiAutoMaxGain:		return ASI_AUTO_MAX_GAIN;
        case AsiAutoMaxExp:		return ASI_AUTO_MAX_EXP;
        case AsiAutoMaxBrightness:	return ASI_AUTO_MAX_BRIGHTNESS;
        case AsiHardwareBin:		return ASI_HARDWARE_BIN;
        case AsiHighSpeedMode:		return ASI_HIGH_SPEED_MODE;
        case AsiCoolerPowerSpec:	return ASI_COOLER_POWER_PERC;
        case AsiTargetTemp:		return ASI_TARGET_TEMP;
        case AsiCoolerOn:		return ASI_COOLER_ON;
        case AsiMonoBin:		return ASI_MONO_BIN;
	}
	std::string	msg = stringprintf("unknown control type %d", type);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

#if 0
static AsiControlType	asitype2type(ASI_CONTROL_TYPE asitype) {
	switch (asitype) {
	case ASI_GAIN:			return AsiGain;
	case ASI_EXPOSURE:		return AsiExposure;
	case ASI_GAMMA:			return AsiGamma;
	case ASI_WB_R:			return AsiWbR;
	case ASI_WB_B:			return AsiWbB;
	case ASI_BRIGHTNESS:		return AsiBrightness;
	case ASI_BANDWIDTHOVERLOAD:	return AsiBandwithoverload;
	case ASI_OVERCLOCK:		return AsiOverclock;
	case ASI_TEMPERATURE:		return AsiTemperature;
	case ASI_FLIP:			return AsiFlip;
	case ASI_AUTO_MAX_GAIN:		return AsiAutoMaxGain;
	case ASI_AUTO_MAX_EXP:		return AsiAutoMaxExp;
	case ASI_AUTO_MAX_BRIGHTNESS:	return AsiAutoMaxBrightness;
	case ASI_HARDWARE_BIN:		return AsiHardwareBin;
	case ASI_HIGH_SPEED_MODE:	return AsiHighSpeedMode;
	case ASI_COOLER_POWER_PERC:	return AsiCoolerPowerSpec;
	case ASI_TARGET_TEMP:		return AsiTargetTemp;
	case ASI_COOLER_ON:		return AsiCoolerOn;
	case ASI_MONO_BIN:		return AsiMonoBin;
	}
}
#endif

/**
 * \brief Get the value of a control
 */
AsiControlValue	AsiCamera::getControlValue(AsiControlType type) {
	long		value;
	ASI_BOOL	pbauto;
	ASI_CONTROL_TYPE	asitype = type2asitype(type);
	int	rc;
	if (ASI_SUCCESS != (rc = ASIGetControlValue(_id, asitype,
		&value, &pbauto))) {
		std::string	msg = stringprintf("%s cannot get control "
			"%d: %s", name().toString().c_str(), type,
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	AsiControlValue	result;
	result.type = type;
	result.value = value;
	result.isauto = (pbauto) ? true : false;
	return result;
}

/**
 * \brief Set the value of a control
 */
void	AsiCamera::setControlValue(const AsiControlValue& controlvalue) {
	ASI_CONTROL_TYPE	type = type2asitype(controlvalue.type);
	long		value = controlvalue.value;
	ASI_BOOL	pbauto = (controlvalue.isauto) ? ASI_TRUE : ASI_FALSE;
	int	rc;
	if (ASI_SUCCESS != (rc = ASISetControlValue(_id, type, value,
		pbauto))) {
		std::string	msg = stringprintf("%s cannot set control %d:"
			" %s", name().toString().c_str(), type,
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
}


/**
 * \brief convert an error code into a string
 */
std::string	AsiCamera::error(int errorcode) {
	switch (errorcode) {
	case ASI_SUCCESS:
		return std::string("no error");
	case ASI_ERROR_INVALID_INDEX:
		return std::string("invalid index");
	case ASI_ERROR_INVALID_ID:
		return std::string("invalid id");
	case ASI_ERROR_INVALID_CONTROL_TYPE:
		return std::string("invalid control type");
	case ASI_ERROR_CAMERA_CLOSED:
		return std::string("camera closed");
	case ASI_ERROR_CAMERA_REMOVED:
		return std::string("camera removed");
	case ASI_ERROR_INVALID_PATH:
		return std::string("invalid path");
	case ASI_ERROR_INVALID_FILEFORMAT:
		return std::string("invalid fileformat");
	case ASI_ERROR_INVALID_SIZE:
		return std::string("invalid size");
	case ASI_ERROR_INVALID_IMGTYPE:
		return std::string("invalid imgtype");
	case ASI_ERROR_OUTOF_BOUNDARY:
		return std::string("outof boundary");
	case ASI_ERROR_TIMEOUT:
		return std::string("timeout");
	case ASI_ERROR_INVALID_SEQUENCE:
		return std::string("invalid sequence");
	case ASI_ERROR_BUFFER_TOO_SMALL:
		return std::string("buffer too small");
	case ASI_ERROR_VIDEO_MODE_ACTIVE:
		return std::string("video mode active");
	case ASI_ERROR_EXPOSURE_IN_PROGRESS:
		return std::string("exposure in progress");
	case ASI_ERROR_GENERAL_ERROR:
		return std::string("general error");
	}
	throw std::range_error("invalid error code");
}

} // namespace asi
} // namespace camera
} // namespace astro
