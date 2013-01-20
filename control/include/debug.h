/*
 * debug.h
 *
 * (c) 2007 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _debug_h
#define _debug_h

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#include <stdarg.h>

#define	DEBUG_NOFILELINE	1
#define DEBUG_ERRNO		2
#define DEBUG_LOG		__FILE__, __LINE__

#ifdef __cplusplus
extern "C" {
#endif

extern int	debuglevel;
extern void	debug(int loglevel, const char *filename, int line,
			int flags, const char *format, ...);
extern void	vdebug(int loglevel, const char *filename, int line,
			int flags, const char *format, va_list ap);

#ifdef __cplusplus
}
#endif

#endif /* _debug_h */
