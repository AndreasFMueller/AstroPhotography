/*
 * stacktrace.cpp -- handler for segmentation faults
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stacktrace.h>
#include <includes.h>

extern "C" void	syslog_stacktrace(int sig) {
	if (sig > 0) {
		syslog(LOG_CRIT, "stacktrace caused by signal %d", sig);
	}
	void	*frames[50];
	int	size = backtrace(frames, sizeof(frames));
	char	**messages = backtrace_symbols(frames, size);
	if (NULL != messages) {
		for (int i = 0; i < size; i++) {
			syslog(LOG_CRIT, "[%d] %s", i, messages[i]);
		}
	} else {
		syslog(LOG_CRIT, "cannot obtain symbolic information");
	}
	if (sig > 0) {
		exit(EXIT_FAILURE);
	}
}

extern "C" void	stderr_stacktrace(int sig) {
	if (sig > 0) {
		fprintf(stderr, "stacktrace caused by signal %d\n", sig);
	}
	void	*frames[50];
	int	size = backtrace(frames, sizeof(frames));
	char	**messages = backtrace_symbols(frames, size);
	if (NULL != messages) {
		for (int i = 0; i < size; i++) {
			fprintf(stderr, "[%d] %s\n", i, messages[i]);
		}
	} else {
		fprintf(stderr, "cannot obtain symbolic information");
	}
	if (sig > 0) {
		exit(EXIT_FAILURE);
	}
}

