/*
 * AsiCamera.cpp -- ASI camera implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiCamera.hh>
#include <utils.h>
#include <AsiCcd.h>
#include <AsiGuidePort.h>
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
AsiCamera::AsiCamera(AsiCameraLocator& locator, int index)
	: Camera(asiCameraName(index)), _locator(locator), _index(index) {
	// if the camera is already open, this constructors should not be
	// called
	if (_locator.isopen(_index)) {
		std::string	msg = stringprintf("%s: internal error, "
			"already open", name().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// open the camera
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open camera idx = %d", index);
	int	rc = ASIOpenCamera(index);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIOpenCamera(%d)",
			rc, index);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("%s: cannot open",
			name().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	_locator.setopen(_index, true);

	// initialize the caomera
	rc = ASIInitCamera(index);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIInitCamera(%d)",
			rc, index);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("%s: cannot initialize",
			name().toString().c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// initialize the mode
	_asi_mode = mode_idle;

	// get information about the CCD
	ASI_CAMERA_INFO camerainfo;
        rc = ASIGetCameraProperty(&camerainfo, _index);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetCameraProperty(%p, %d)",
			rc, &camerainfo, _index);
	}
        if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("%s: cannot get props: %s",
			name().toString().c_str(), error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// set common variables depending on the camera info
	_hasGuidePort = (camerainfo.ST4Port) ? true : false;
	_isColor = (camerainfo.IsColorCam) ? true : false;
	_hasCooler = (camerainfo.IsCoolerCam) ? true : false;
	_id = camerainfo.CameraID;
	_userFriendlyName = std::string(camerainfo.Name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera idx = %d has id = %d",
		_index, _id);
	ImageSize	size(camerainfo.MaxWidth, camerainfo.MaxHeight);

	// construct a CcdInfo object for each image format
	std::vector<std::string>	ccdnames
		= _locator.imgtypes(index);
	std::vector<std::string>::const_iterator	i;
	for (i = ccdnames.begin(); i != ccdnames.end(); i++) {
		// construct the name for this 
		DeviceName	ccdname = name().child(DeviceName::Ccd, *i);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding ccd %s",
			ccdname.toString().c_str());
		CcdInfo	info(ccdname, size, 0);

		// pixel size
		info.pixelwidth(camerainfo.PixelSize * 1e-6);
		info.pixelheight(camerainfo.PixelSize * 1e-6);

		// exposure time range
		info.minexposuretime(0.0001); // tested with ASI120MM-S
		info.maxexposuretime(3600);

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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "have now %d infos",
			ccdinfo.size());
	}

	// read the bandwith limit property and set the control accordingly
	Properties	properties(name());
	if (properties.hasProperty(std::string("bandwidth"))) {
		long	bandwidth = std::stol(
			properties.getProperty(std::string("bandwidth")));
		if ((bandwidth <= 100) && (bandwidth > 0)) {
			rc = ASISetControlValue(_id, ASI_BANDWIDTHOVERLOAD,
				bandwidth, ASI_FALSE);
			if (rc != ASI_SUCCESS) {
				debug(LOG_ERR, DEBUG_LOG, 0, "could not set "
					"bandwidth limit %ld: %d",
					bandwidth, rc); 
			}
		}
	}
	
}

/**
 * \brief Destroy the AsiCamera
 */
AsiCamera::~AsiCamera() {
	int	rc;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "close camera %d (id = %d)", _index, _id);
	rc = ASICloseCamera(_index);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASICloseCamera(%d)",
			rc, _index);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("%s cannot close camera: %s",
			name().toString().c_str(), error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		return;
	}
	_locator.setopen(_index, false);
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
bool    AsiCamera::hasGuidePort() const {
	return _hasGuidePort;
}

/**
 * \brief Get a guider port
 */
GuidePortPtr	AsiCamera::getGuidePort0() {
	return GuidePortPtr(new AsiGuidePort(*this));
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	int	n;
	int	rc;
	rc = ASIGetNumOfControls(_id, &n);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetNumOfControls(%d, %d)",
			rc, _id, n);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("%s cannot get controls: %s",
			name().toString().c_str(),
			AsiCamera::error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	for (int i = 0; i < n; i++) {
		ASI_CONTROL_CAPS	caps;
		int	rc;
		rc = ASIGetControlCaps(_id, i, &caps);
		if (Asi_Debug_Apicalls) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"%d = ASIGetControlCaps(%d, %d, %d)",
				rc, _id, i, caps);
		}
		if (ASI_SUCCESS != rc) {
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
	int	rc = ASIGetControlCaps(id, control_index, caps);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetControlCaps(%d, %d, %p)",
			rc, id, control_index, caps);
	}
	if (ASI_SUCCESS != rc) {
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	ASI_CONTROL_CAPS	caps;
	getControlCapabilities(_id, control_index, &caps);
	return std::string(caps.Description);
}

/**
 * \brief Get the description of a control by name
 */
std::string     AsiCamera::controlDescription(const std::string& controlname) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	return controlDescription(controlIndex(controlname));
}

/**
 * \brief get whether a control is writabl by index
 */
bool    AsiCamera::controlWritable(int control_index) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
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
	case AsiFanOn:			return ASI_FAN_ON;
	case AsiPatternAdjust:		return ASI_PATTERN_ADJUST;
	case AsiAntiDewHeater:		return ASI_ANTI_DEW_HEATER;
	}
	std::string	msg = stringprintf("unknown control type %d", type);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

#if 1
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
	case ASI_FAN_ON:		return AsiFanOn;
	case ASI_PATTERN_ADJUST:	return AsiPatternAdjust;
	case ASI_ANTI_DEW_HEATER:	return AsiAntiDewHeater;
	}
}
#endif

/**
 * \brief Get the value of a control
 */
AsiControlValue	AsiCamera::getControlValue(AsiControlType type) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	long		value;
	ASI_BOOL	pbauto;
	ASI_CONTROL_TYPE	asitype = type2asitype(type);
	int	rc;
	rc = ASIGetControlValue(_id, asitype, &value, &pbauto);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetControlValue(%d, %d, %ld, %s)",
			rc, _id, asitype, value, (pbauto) ? "TRUE" : "FALSE");
	}
	if (ASI_SUCCESS != rc) {
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
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	ASI_CONTROL_TYPE	type = type2asitype(controlvalue.type);
	long		value = controlvalue.value;
	ASI_BOOL	pbauto = (controlvalue.isauto) ? ASI_TRUE : ASI_FALSE;
	int	rc;
	rc = ASISetControlValue(_id, type, value, pbauto);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASISetControlValue(%d, %d, %ld, %s)",
			rc, _id, type, value, (pbauto) ? "TRUE" : "FALSE");
	}
	if (ASI_SUCCESS != rc) {
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

/**
 * \brief Set the ROI
 */
void	AsiCamera::setROIFormat(const roi_t roi) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	int	iBin = roi.mode.x();
	ASI_ERROR_CODE	rc = ASISetROIFormat(_id, roi.size.width(),
				roi.size.height(), iBin, roi.img_type);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASISetROIFormat(%d, %d, %d, %d, %d)", rc, _id, 
			roi.size.width(), roi.size.height(), iBin,
			roi.img_type);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot set ROI %s, %s, %d: %s",
			roi.size.toString().c_str(),
			roi.mode.toString().c_str(), roi.img_type,
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
}

/**
 * \brief Get the current ROI
 */
AsiCamera::roi_t   AsiCamera::getROIFormat() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	int	iWidth, iHeight, iBin;
	ASI_IMG_TYPE	Img_type;
	ASI_ERROR_CODE	rc = ASIGetROIFormat(_id, &iWidth, &iHeight, &iBin,
		&Img_type);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetROIFormat(%d, %d, %d, %d, %d)",
			_id, iWidth, iHeight, iBin, Img_type);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot get ROI: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	roi_t	roi;
	roi.size = ImageSize(iWidth, iHeight);
	roi.mode = Binning(iBin, iBin);
	roi.img_type = Img_type;
	return roi;
}

/**
 * \brief Set the start position
 */
void    AsiCamera::setStartPos(const ImagePoint& point) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	ASI_ERROR_CODE	rc = ASISetStartPos(_id, point.x(), point.y());
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASISetStartPos(%d, %d, %d)",
			rc, _id, point.x(), point.y());
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot set start position "
			"%s: %s", point.toString().c_str(), error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start position set to %d,%d",
		point.x(), point.y());
}

/**
 * \brief Get the start position
 */
ImagePoint      AsiCamera::getStartPos() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	int	iStartX, iStartY;
	ASI_ERROR_CODE	rc = ASIGetStartPos(_id, &iStartX, &iStartY);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetStartPos(%d, %d, %d)",
			rc, _id, iStartX, iStartY);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot get start pos: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	return ImagePoint(iStartX, iStartY);
}

/**
 * \brief Get the number of dropped frames
 */
unsigned long   AsiCamera::getDroppedFrames() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	int	iDropFrames;
	ASI_ERROR_CODE	rc = ASIGetDroppedFrames(_id, &iDropFrames);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetDroppedFrames(%d, %d)",
			rc, _id, iDropFrames);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot get dropped: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	return iDropFrames;
}

/**
 * \brief start exposure
 */
void	AsiCamera::startExposure(bool isdark) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	// make sure the camera is idle
	if (_asi_mode != mode_idle) {
		std::string	msg = stringprintf("camera not idle: %d",
			_asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// start the exposure
	ASI_BOOL	isDark = (isdark) ? ASI_TRUE : ASI_FALSE;
	ASI_ERROR_CODE	rc = ASIStartExposure(_id, isDark);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIStartExposure(%d, %s)",
			rc, _id, (isDark == ASI_TRUE) ? "TRUE" : "FALSE");
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot start exposure: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	_asi_mode = mode_exposure;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera %d: exposure started: %d", _id,
		getExpStatus());
}

/**
 * \brief stop exposure
 */
void	AsiCamera::stopExposure() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	// make sure the camera is in stream mode
	if (_asi_mode != mode_stream) {
		std::string	msg = stringprintf("camera not in stream mode: %d",
			_asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// stop the exposure
	ASI_ERROR_CODE	rc = ASIStopExposure(_id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIStopExposure(%d)", rc, _id);
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot stop exposure: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	_asi_mode = mode_idle;
}

/**
 * \brief Get the exposure status
 */
ASI_EXPOSURE_STATUS	AsiCamera::getExpStatus() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	// check that we are in the correct mode
	switch (_asi_mode) {
	case mode_idle:
		if (Asi_Debug_State) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "asi_mode = idle");
		}
		return ASI_EXP_IDLE;
	case mode_stream:
		{
		std::string	msg = stringprintf("camera not in exposure "
			"mode: %d", _asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		}
	default:
		break;
	}
	// actually get the exposure status
	ASI_EXPOSURE_STATUS	ExpStatus;
	ASI_ERROR_CODE	rc = ASIGetExpStatus(_id, &ExpStatus);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIGetExpStatus(%d, %d)",
			rc, _id, ExpStatus);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot get exp status: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	if (Asi_Debug_State) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera%d: exposure status: %d",
			_id, ExpStatus);
	}
	// reset the mode if the exposure failed
	if (ExpStatus == ASI_EXP_FAILED) {
		_asi_mode = mode_idle;
	}
	return ExpStatus;
}

/**
 * \brief retrieve the data
 */
void	AsiCamera::getDataAfterExp(unsigned char *pBuffer, long lBuffSize) {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	if (_asi_mode != mode_exposure) {
		std::string	msg = stringprintf("%s: not in exposure mode: %d",
			name().toString().c_str(), _asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image buffer[%ld]@%p", lBuffSize,
		pBuffer);
	ASI_ERROR_CODE	rc = ASIGetDataAfterExp(_id, pBuffer, lBuffSize);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetDataAfterExp(%d, %p, %ld",
			rc, _id, pBuffer, lBuffSize);
	}
	_asi_mode = mode_idle;
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot get exp data: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
}

/**
 * \brief Start the video stream
 */
void    AsiCamera::startVideoCapture() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	// make sure the camera is idle
	if (_asi_mode != mode_idle) {
		std::string	msg = stringprintf("camera not idle: %d",
			_asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	// start video capture
	ASI_ERROR_CODE	rc = ASIStartVideoCapture(_id);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIStartVideoCapture(%d)",
			rc, _id);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot start video: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	_asi_mode = mode_stream;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "video capture started");
}

/**
 * \brief Stop the video stream
 */
void    AsiCamera::stopVideoCapture() {
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	// make sure the camera is in stream mode
	if (_asi_mode != mode_stream) {
		std::string	msg = stringprintf("camera not in stream mode: %d",
			_asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ASI_ERROR_CODE	rc = ASIStopVideoCapture(_id);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIStopVideoCapture(%d)",
			rc, _id);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot stop video: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
	_asi_mode = mode_idle;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "video capture stopped");
}

/**
 * \brief retrieve the video data
 */
void	AsiCamera::getVideoData(unsigned char *pBuffer, long lBuffSize,
	int iWaitms) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting video data, timeout=%dms",
		iWaitms);
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	if (_asi_mode != mode_stream) {
		std::string	msg = stringprintf("%s: not in stream mode: %d",
			name().toString().c_str(), _asi_mode);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ASI_ERROR_CODE	rc = ASIGetVideoData(_id, pBuffer, lBuffSize, iWaitms);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"%d = ASIGetVideoData(%d, %p, %ld, %d)",
			rc, _id, pBuffer, lBuffSize, iWaitms);
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot get video data: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
}

/**
 * \brief Convert AsiCamera direction to ASI direction
 */
static ASI_GUIDE_DIRECTION	dir2dir(AsiCamera::direction_t dir) {
	switch (dir) {
	case AsiCamera::asi_guide_north:return ASI_GUIDE_NORTH;
	case AsiCamera::asi_guide_south:return ASI_GUIDE_SOUTH;
	case AsiCamera::asi_guide_east:	return ASI_GUIDE_EAST;
	case AsiCamera::asi_guide_west:	return ASI_GUIDE_WEST;
	}
	std::string	msg = stringprintf("unknown direction: %d", dir);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Convert AsiCamera direction to string
 */
static std::string	dir2string(AsiCamera::direction_t dir) {
	switch (dir) {
	case AsiCamera::asi_guide_north:return std::string("north");
	case AsiCamera::asi_guide_south:return std::string("south");
	case AsiCamera::asi_guide_east:	return std::string("east");
	case AsiCamera::asi_guide_west:	return std::string("west");
	}
	std::string	msg = stringprintf("unknown direction: %d", dir);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

/**
 * \brief Turn pulse guide direction on
 */
void    AsiCamera::pulseGuideOn(direction_t dir) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning on pulse dir %s",
		dir2string(dir).c_str());
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	ASI_GUIDE_DIRECTION	direction = dir2dir(dir);
	ASI_ERROR_CODE	rc = ASIPulseGuideOn(_id, direction);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIPulseGuideOn(%d, %s)",
			rc, _id, dir2string(dir).c_str());
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot pulse on: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
}

/**
 * \brief Turn pulse guide direction off
 */
void    AsiCamera::pulseGuideOff(direction_t dir) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "turning off pulse dir %s",
		dir2string(dir).c_str());
	std::unique_lock<std::recursive_mutex>	lock(_api_mutex);
	ASI_GUIDE_DIRECTION	direction = dir2dir(dir);
	ASI_ERROR_CODE	rc = ASIPulseGuideOff(_id, direction);
	if (Asi_Debug_Apicalls) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d = ASIPulseGuideOff(%d, %s)",
			rc, _id, dir2string(dir).c_str());
	}
	if (ASI_SUCCESS != rc) {
		std::string	msg = stringprintf("cannot pulse off: %s",
			error(rc).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw AsiApiException(rc, msg);
	}
}


} // namespace asi
} // namespace camera
} // namespace astro
