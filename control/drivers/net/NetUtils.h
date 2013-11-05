/*
 * NetUtils.h -- utilities for the net module
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _NetUtils_h
#define _NetUtils_h

#include <AstroDevice.h>
#include <AstroUtils.h>

namespace astro {
namespace camera {
namespace net {

astro::DeviceName	devname2netname(const astro::DeviceName& devname);
astro::DeviceName	devname2netname(const std::string& devname);

astro::DeviceName	netname2devname(const astro::DeviceName& netname);

} // namespace net
} // namespace camera
} // namespace astro

#endif /* _NetUtils_h */
