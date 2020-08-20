/*
 * ContextHolder.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>

namespace astro {
namespace usb {

ContextHolder::ContextHolder() {
	_context = NULL;
	enum libusb_error	rc = (enum libusb_error)libusb_init(&_context);
	if (rc) {
		throw USBError(rc);
	}
}

ContextHolder::~ContextHolder() {
	libusb_exit(_context);
}

void	ContextHolder::debug(int level) {
	if (level > 4) {
		level = 4;
	}
	if (level < 0) {
		level = 0;
	}
#ifdef LIBUSB_OPTION_LOG_LEVEL
	libusb_set_option(_context, LIBUSB_OPTION_LOG_LEVEL, level);
#else
	libusb_set_debug(_context, level);
#endif
}

} // namespace usb
} // namespace astro
