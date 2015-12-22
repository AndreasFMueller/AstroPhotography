/*
 * QhyUtils.cpp -- utility functions for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace qhy {

#define QHY_VENDOR_ID	0x1618
#define QHY_VENDOR_NAME	"qhy"

/**
 * \brief Construct a QHY name based on the USB device ptr
 *
 * \param deviceptr	USB device ptr to construct the 
 */
QhyName::QhyName(astro::usb::DevicePtr deviceptr)
	: DeviceNameUSB(QHY_VENDOR_NAME, QHY_VENDOR_ID, deviceptr) {
}

/**
 * \brief Construct a QHY name basd on the device name
 */
QhyName::QhyName(const DeviceName& devicename)
	: DeviceNameUSB(QHY_VENDOR_NAME, QHY_VENDOR_ID, devicename) {
}

} // namespace qhy
} // namespace camera
} // namespace astro
