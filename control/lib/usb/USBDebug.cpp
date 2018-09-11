/*
 * USBDebug.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <USBDebug.h>

namespace astro {
namespace usb {

static bool	usbdebug_enabled = true;

/**
 * \brief Enable USB debugging
 */
void	USBdebugEnable() {
	usbdebug_enabled = true;
}

/**
 * \brief Disable USB debugging
 */
void	USBdebugDisable() {
	usbdebug_enabled = false;
}

/**
 * \brief USB debug message
 */
extern "C" void	USBdebug(int loglevel, const char *filename, int line,
		int flags, const char *format, ...) {
	va_list ap;
	// forward to the normal debug() function if not a debug message
	// or if debugging is enabled
	if ((debuglevel < LOG_DEBUG) || (usbdebug_enabled)) {
		va_start(ap, format);
		vdebug(loglevel, filename, line, flags, format, ap);
		va_end(ap);
	}
}

} // namespace usb
} // namespace astro
