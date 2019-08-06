/*
 * AsiCamera.hh -- ASI camera class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AsiCamera_hh
#define _AsiCamera_hh

#include <AsiLocator.h>
#include <AstroCamera.h>
#include <ASICamera2.h>

namespace astro {
namespace camera {
namespace asi {

#define	Asi_Debug_State		1
#define	Asi_Debug_Apicalls	0

typedef enum AsiControlType_ {
	AsiGain = 0,
	AsiExposure,
	AsiGamma,
	AsiWbR,
	AsiWbB,
	AsiBrightness,
	AsiBandwithoverload,
	AsiOverclock,
	AsiTemperature,
	AsiFlip,
	AsiAutoMaxGain,
	AsiAutoMaxExp,
	AsiAutoMaxBrightness,
	AsiHardwareBin,
	AsiHighSpeedMode,
	AsiCoolerPowerSpec,
	AsiTargetTemp,
	AsiCoolerOn,
	AsiMonoBin,
	AsiFanOn,
	AsiPatternAdjust,
	AsiAntiDewHeater
} AsiControlType;

class AsiControlValue {
public:
	AsiControlType	type;
	long	value;
	bool	isauto;
};

class AsiApiException : public std::runtime_error {
	ASI_ERROR_CODE	_e;
public:
	ASI_ERROR_CODE	error_code() const { return _e; }
	AsiApiException(ASI_ERROR_CODE e, const std::string& cause)
		: std::runtime_error(cause), _e(e) {
	}
};

/**
 * \brief AsiCamera class
 */
class AsiCamera : public Camera {
	AsiCameraLocator&	_locator;
	std::recursive_mutex	_api_mutex;
	int	_id;
	int	_index;
	std::string	_userFriendlyName;
public:
	int	id() const { return _id; }
	int	index() const { return _index; }
private:
	bool	_hasCooler;
public:
	AsiCamera(AsiCameraLocator&, int index);
	~AsiCamera();

	// prevent copying of the camera class
private:
	AsiCamera(const AsiCamera& other);
	AsiCamera&	operator=(const AsiCamera& other);

protected:
	virtual CcdPtr	getCcd0(size_t id);

private:
	bool	_hasGuidePort;
public:
	virtual bool	hasGuidePort() const;
protected:
	virtual GuidePortPtr	getGuidePort0();

private:
	bool	_isColor;
public:
	bool	isColor() const;
private:
	image::MosaicType	_mosaic;
public:
	image::MosaicType	mosaic() const { return _mosaic; }

private:
	float	_settemperature;
public:
	float	settemperature() const { return _settemperature; }
	void	settemperature(float s) { _settemperature = s; }
	
	// error converstion
	static std::string	error(int errorcode);
	// properties
	int	controlIndex(const std::string& controlname);
	long	controlMax(int control_index);
	long	controlMax(const std::string& controlname);
	long	controlMin(int control_index);
	long	controlMin(const std::string& controlname);
	long	controlDefault(int control_index);
	long	controlDefault(const std::string& controlname);
	std::string	controlName(int control_index);
	std::string	controlName(const std::string& controlname);
	std::string	controlDescription(int control_index);
	std::string	controlDescription(const std::string& controlname);
	bool	controlWritable(int control_index);
	bool	controlWritable(const std::string& controlname);

	AsiControlValue	getControlValue(AsiControlType type);
	void	setControlValue(const AsiControlValue& value);

	typedef enum asi_mode_e {
		mode_idle, mode_exposure, mode_stream
	} asi_mode_t;
private:
	std::recursive_mutex	_mode_lock;
	asi_mode_t	_asi_mode;
public:
	asi_mode_t	asi_mode() const { return _asi_mode; }

	// get/set ROI
	typedef struct roi_s {
		ImageSize	size;
		Binning		mode;
		ASI_IMG_TYPE	img_type;
	} roi_t;
	void	setROIFormat(const roi_t roi);
	roi_t	getROIFormat();

	// get/set the start position
	void	setStartPos(const ImagePoint& point);
	ImagePoint	getStartPos();

	// get dropped frames
	unsigned long	getDroppedFrames();

	// normal exposure
	void	startExposure(bool isDark);
	void	stopExposure();
	ASI_EXPOSURE_STATUS	getExpStatus();
	void	getDataAfterExp(unsigned char *pBuffer, long lBuffSize);

	// video capture
	void	startVideoCapture();
	void	stopVideoCapture();
	void	getVideoData(unsigned char *pBuffer, long iBuffSize,
			int iWaitms);

	// guiding
	typedef enum direction_e {
		asi_guide_north = 0,
		asi_guide_south,
		asi_guide_east,
		asi_guide_west
	} direction_t;
	void	pulseGuideOn(direction_t dir);
	void	pulseGuideOff(direction_t dir);

	virtual std::string	userFriendlyName() const {
		return _userFriendlyName;
	}
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiCamera_hh */
