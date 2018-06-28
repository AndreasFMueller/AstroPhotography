/*
 * USBError.cpp -- constructor for USB error
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>

namespace astro {
namespace usb {

USBError::USBError(enum libusb_error errcode)
	: std::runtime_error(libusb_strerror(errcode)) {
}

USBError::USBError(const std::string& s) : std::runtime_error(s.c_str()) {
}

} // namespace usb
} // namespace astro
