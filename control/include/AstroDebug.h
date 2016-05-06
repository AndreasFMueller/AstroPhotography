/*
 * AstroDebug.h
 *
 * (c) 2007 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroDebug_h
#define _AstroDebug_h

#include <syslog.h>
#include <stdio.h>
#include <stdarg.h>

#define	DEBUG_NOFILELINE	1
#define DEBUG_ERRNO		2
#define DEBUG_LOG		__FILE__, __LINE__

#ifdef __cplusplus
extern "C" {
#endif

extern int	debuglevel;
extern int	debugtimeprecision;
extern int	debugthreads;
extern void	debug(int loglevel, const char *filename, int line,
			int flags, const char *format, ...);
extern void	vdebug(int loglevel, const char *filename, int line,
			int flags, const char *format, va_list ap);

extern void	debug_set_ident(const char *ident);
extern void	debug_syslog(int facility);
extern void	debug_stderr();
extern void	debug_fd(int fd);
extern int	debug_file(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* _AstroDebug_h */
