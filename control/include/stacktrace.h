/*
 * stacktrace.h -- handler for segmentation faults
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <syslog.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void	syslog_stacktrace(int sig);
extern void	stderr_stacktrace(int sig);

#ifdef __cplusplus
}
#endif
