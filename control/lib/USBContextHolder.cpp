/*
 * ContextHolder.cpp -- ContextHolder implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <AstroDebug.h>

namespace astro {
namespace usb {

/**
 * \brief Create the context holder
 */
ContextHolder::ContextHolder() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating USB context");
	int     rc = libusb_init(&_context);
	if (rc != 0) {
		throw USBError(libusb_error_name(rc));
	}
	const libusb_version    *version = libusb_get_version();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "libusb version: %hu.%hu.%hu",
		version->major, version->minor, version->micro);
}

/**
 * \brief Destroy the context holder
 */
ContextHolder::~ContextHolder() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying USB context");
	libusb_exit(_context);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "USB context destroyed");
}

} // namespace usb
} // namespace astro
