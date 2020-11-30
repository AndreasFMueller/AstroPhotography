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

#define QHY_VENDOR_ID	0x1618

/**
 * \brief Class to encapsulate all the nameing logic
 *
 * This class encapsulates all the name related functions used in the QHY
 * driver
 */
class Qhy2Name : public device::DeviceNameUSB {
public:
	Qhy2Name(astro::usb::DevicePtr deviceptr);
	Qhy2Name(const astro::DeviceName& devicename);
};

} // namespace qhy2
} // namespace camera
} // namespace astro

#endif /* _Qhy2Utils_h */
