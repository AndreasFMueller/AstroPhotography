/*
 * Qhy2Utils.h -- utilities for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Qhy2Utils_h
#define _Qhy2Utils_h

#include <AstroUSB.h>
#include <AstroDevice.h>
#include <DeviceNameUSB.h>

namespace astro {
namespace camera {
namespace qhy2 {

/**
 * \brief Exception thrown in the driver
 */
class Qhy2Error : public std::runtime_error {
	int	_error;
public:
	int	error() const { return _error; }
	Qhy2Error(const std::string& cause, int error)
		: std::runtime_error(cause), _error(error) { }
	static std::string	err2string(int error);
};

/**
 * \brief Class to encapsulate all the nameing logic
 *
 * This class encapsulates all the name related functions used in the QHY
 * driver
 */
class Qhy2Name : public astro::DeviceName {
	int		_qhyindex;
	std::string	_qhyname;
	static std::vector<std::string>	qhybasename();
public:
	const std::string&	qhyname() const { return _qhyname; }
	int	qhyindex() const { return _qhyindex; }
	Qhy2Name(int qhyindex);
	Qhy2Name(const std::string& qhyname);
	DeviceName	cameraname() const;
	DeviceName	coolername() const;
	DeviceName	filterwheelname() const;
	DeviceName	guideportname() const;
	DeviceName	ccdname(const std::string& ccd) const;
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Utils_h */
