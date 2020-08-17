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
	libusb_set_option(_context, LIBUSB_OPTION_LOG_LEVEL, level);
}

} // namespace usb
} // namespace astro
