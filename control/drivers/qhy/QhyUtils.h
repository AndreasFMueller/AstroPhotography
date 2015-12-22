/*
 * QhyUtils.h -- utilities for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QhyUtils_h
#define _QhyUtils_h

#include <AstroUSB.h>
#include <AstroDevice.h>
#include <DeviceNameUSB.h>

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
class QhyName : public device::DeviceNameUSB {
public:
	QhyName(astro::usb::DevicePtr deviceptr);
	QhyName(const astro::DeviceName& devicename);
};

} // namespace qhy
} // namespace camera
} // namespace astro

#endif /* _QhyUtils_h */
