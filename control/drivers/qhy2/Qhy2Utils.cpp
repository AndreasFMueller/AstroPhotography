/*
 * Qhy2Utils.cpp -- utility functions for QHY cameras
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Qhy2Utils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace qhy2 {

#define QHY_VENDOR_ID	0x1618
#define QHY_VENDOR_NAME	"qhy2"

/**
 * \brief Construct a QHY name based on the USB device ptr
 *
 * \param deviceptr	USB device ptr to construct the 
 */
Qhy2Name::Qhy2Name(astro::usb::DevicePtr deviceptr)
	: DeviceNameUSB(QHY_VENDOR_NAME, QHY_VENDOR_ID, deviceptr) {
}

/**
 * \brief Construct a QHY name basd on the device name
 */
Qhy2Name::Qhy2Name(const DeviceName& devicename)
	: DeviceNameUSB(QHY_VENDOR_NAME, QHY_VENDOR_ID, devicename) {
}

} // namespace qhy2
} // namespace camera
} // namespace astro
