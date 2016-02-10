/*
 * OthelloUtil.h -- utilities for the othello driver
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _OthelloUtil_h
#define _OthelloUtil_h

#include <AstroDevice.h>
#include <AstroUSB.h>
#include <string>

#define OTHELLO_VENDOR_ID	0xF055

#define OTHELLO_GUIDERPORT_ID	0x1234
#define OTHELLO_FOCUSER_ID	0x1235

namespace astro {
namespace camera {
namespace othello {

std::string	othelloname(astro::usb::DevicePtr& deviceptr);
DeviceName	othellodevname(astro::usb::DevicePtr& deviceptr);

} // namespace othello
} // namespace camera
} // namespace astro

#endif /* _OthelloUtil_h */
