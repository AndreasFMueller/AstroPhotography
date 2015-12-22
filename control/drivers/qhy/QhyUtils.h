/*
 * QhyUtils.h -- utilities for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QhyUtils_h
#define _QhyUtils_h

#include <AstroUSB.h>

namespace astro {
namespace camera {
namespace qhy {

#define QHY_VENDOR_ID	0x1618

extern std::string	qhyname(astro::usb::DevicePtr deviceptr);
extern void	qhyparse(const std::string& name, int& busnumber,
			int& deviceaddress);

} // namespace qhy
} // namespace camera
} // namespace astro

#endif /* _QhyUtils_h */
