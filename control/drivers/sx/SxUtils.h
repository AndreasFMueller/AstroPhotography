/*
 * SxUtils.h -- utilities for 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SxUtils_h
#define _SxUtils_h

#include <stdexcept>
#include <AstroUSB.h>
#include <DeviceNameUSB.h>

namespace astro {
namespace camera {
namespace sx {


class SxError : public std::runtime_error {
public:
	SxError(const char *cause);
};

#define	SX_VENDOR_ID	0x1278

class SxName : public device::DeviceNameUSB {
public:
	SxName(usb::DevicePtr deviceptr);
	SxName(const DeviceName& devicename);
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxUtils_h */
