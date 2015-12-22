/*
 * QhyUtils.h -- utilities for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QhyUtils_h
#define _QhyUtils_h

#include <AstroUSB.h>
#include <AstroDevice.h>

namespace astro {
namespace camera {
namespace qhy {

#define QHY_VENDOR_ID	0x1618

/**
 * \brief Class to encapsulate all the nameing logic
 *
 * This class encapsulates all the name related functions used in the QHY
 * driver
 */
class QhyName {
	int	_busnumber;
	int	_deviceaddress;
public:
	int	busnumber() const { return _busnumber; }
	int	deviceaddress() const { return _deviceaddress; }
private:
	std::string	iproduct;
	unsigned short	idvendor;
	unsigned short	idproduct;
	std::string	serial;

	void	parse(const std::string& name);
	std::string	unparse() const;
	bool	matches(const DeviceName& other, DeviceName::device_type type);
public:
	QhyName(astro::usb::DevicePtr deviceptr);
	QhyName(const astro::DeviceName& devicename);
	DeviceName	name(DeviceName::device_type type) const;
	DeviceName	name(DeviceName::device_type type,
				const std::string& path) const;
	DeviceName	name(DeviceName::device_type type,
				const std::vector<std::string>& path) const;
	std::string	stringname(DeviceName::device_type type) const;
	std::string	stringname(DeviceName::device_type type,
				const std::string& path) const;
	std::string	stringname(DeviceName::device_type type,
				const std::vector<std::string>& path) const;
	DeviceName	cameraname() const;
	DeviceName	ccdname() const;
	DeviceName	coolername() const;
	DeviceName	guiderportname() const;
	bool	isCamera(const DeviceName& other);
	bool	isCcd(const DeviceName& other);
	bool	isCooler(const DeviceName& other);
	bool	isGuiderport(const DeviceName& other);
};

} // namespace qhy
} // namespace camera
} // namespace astro

#endif /* _QhyUtils_h */
