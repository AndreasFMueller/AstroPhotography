/*
 * AsiCamera.hh -- ASI camera class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AsiCamera_hh
#define _AsiCamera_hh

#include <AstroCamera.h>
#include <AsiCamera2.h>

namespace astro {
namespace camera {
namespace asi {

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
	AsiMonoBin
} AsiControlType;

class AsiControlValue {
public:
	AsiControlType	type;
	long	value;
	bool	isauto;
};


/**
 * \brief AsiCamera class
 */
class AsiCamera : public Camera {
	int	_index;
public:
	int	index() const { return _index; }
private:
	bool	_hasCooler;
public:
	AsiCamera(int index);
	~AsiCamera();

	// prevent copying of the camera class
private:
	AsiCamera(const AsiCamera& other);
	AsiCamera&	operator=(const AsiCamera& other);

protected:
	virtual CcdPtr	getCcd0(size_t id);

private:
	bool	_hasGuiderPort;
public:
	virtual bool	hasGuiderPort() const;
protected:
	virtual GuiderPortPtr	getGuiderPort0();

private:
	bool	_isColor;
public:
	bool	isColor() const;
	// error converstion
	static std::string	error(int errorcode);
	// properties
	int	controlIndex(const std::string& controlname);
	long	controlMax(int control_index);
	long	proeprtyMax(const std::string& controlname);
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
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiCamera_hh */
