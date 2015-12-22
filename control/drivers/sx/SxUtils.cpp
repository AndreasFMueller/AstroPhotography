/*
 * SxUtils.cpp -- starlight express utilities 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxUtils.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <DeviceNameUSB.h>

using namespace astro::usb;

namespace astro {
namespace camera {
namespace sx {

SxError::SxError(const char *cause) : std::runtime_error(cause) {
}

#define SX_MODULE_NAME	"sx"
#define SX_VENDOR_ID	0x1278

/**
 * \brief Construct an SxName from a USB device ptr
 */
SxName::SxName(usb::DevicePtr deviceptr)
	: DeviceNameUSB(SX_MODULE_NAME, SX_VENDOR_ID, deviceptr) {
}

/**
 * \brief Construct an SxName from a DeviceName
 */
SxName::SxName(const DeviceName& devicename)
	: DeviceNameUSB(SX_MODULE_NAME, SX_VENDOR_ID, devicename) {
}

} // namespace sx
} // namespace camera
} // namespace astro
