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
#include "sx.h"

namespace astro {
namespace camera {
namespace sx {

extern std::string	command_name(sx_command_t command);

class SxError : public std::runtime_error {
public:
	SxError(const char *cause);
};

class SxName : public device::DeviceNameUSB {
public:
	SxName(usb::DevicePtr deviceptr);
	SxName(const DeviceName& devicename);
};

extern std::string     wchar2string(const wchar_t *w);

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxUtils_h */
