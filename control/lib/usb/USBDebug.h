/*
 * USBDebug.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rappreswil
 */
#ifndef _USBDebug_h
#define _USBDebug_h

#include <stdarg.h>
#include <AstroDebug.h>

namespace astro {
namespace usb {

extern "C" void	USBdebug(int loglevel, const char *filename, int line,
			int flags, const char *format, ...);

} // namespace usb
} // namespace astro

#endif /* _USBDebug_h */
