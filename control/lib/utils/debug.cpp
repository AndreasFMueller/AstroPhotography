//
// debug.cpp
//
// (c) 2007 Prof Dr Andreas Mueller, Hochschule Rapperswil
// $Id: debug.cpp,v 1.3 2008/12/05 18:08:25 afm Exp $
//
#include <AstroDebug.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <iostream>
#include <sstream>
#include <map>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif /* HAVE_SYSLOG_H */

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

int	debuglevel = LOG_ERR;
int	debugtimeprecision = 0;
int	debugthreads = 0;

int	debugmaxlines = 0;
int	debugnfiles = 0;
static char	*logfilename = NULL;

#define DEBUG_STDERR	0
#define DEBUG_FD	1
#define	DEBUG_SYSLOG	2

static int	debug_destination = DEBUG_STDERR;
static char	*debug_ident = NULL;

#define	DEBUG_IDENT	((debug_ident) ? debug_ident : "astro")

extern "C" void	debug(int loglevel, const char *file, int line,
	int flags, const char *format, ...) {
	va_list ap;
	if (loglevel > debuglevel) { return; }
	va_start(ap, format);
	vdebug(loglevel, file, line, flags, format, ap);
	va_end(ap);
}

extern "C" void debug_set_ident(const char *ident) {
	if (NULL == ident) {
		return;
	}
	if (debug_ident) {
		free(debug_ident);
		debug_ident = NULL;
	}
	debug_ident = strdup(ident);
}

extern "C" void	debug_syslog(int facility) {
	openlog(DEBUG_IDENT, LOG_NDELAY, facility);
	debug_destination = DEBUG_SYSLOG;
	logfilename = NULL;
}

extern "C" void	debug_stderr() {
	debug_destination = DEBUG_STDERR;
	logfilename = NULL;
}

static int	debug_filedescriptor = -1;

extern "C" void debug_fd(int fd) {
	logfilename = NULL;
	if (debug_filedescriptor >= 0) {
		close(debug_filedescriptor);
		debug_filedescriptor = -1;
	}
	debug_filedescriptor = fd;
	debug_destination = DEBUG_FD;
}

static int	linecounter = 0;

extern "C" int debug_file(const char *filename) {
	// find out whether the file exists
	struct stat	sb;
	if (stat(filename, &sb) < 0) {
		linecounter = 0;
	} else {
		linecounter = debugmaxlines + 1;
	}

	// create or open the new file
	int	fd = open(filename, O_CREAT | O_WRONLY, 0666);
	if (fd < 0) {
		return -1;
	}
	debug_fd(fd);
	logfilename = strdup(filename);
	return 0;
}

static void	rotate_logfile() {
	// if the log file name is not known, we cannot rotate the log file
	if (NULL == logfilename) {
		return;
	}

	// how many positions
	int	positions = 0;
	int	d = debugnfiles;
	do {
		d = d / 10;
		positions++;
	} while (d);

	// rotate the old log files
	for (int n = debugnfiles; n >= 0; n--) {
		char	from[MAXPATHLEN + 1];
		snprintf(from, sizeof(from), "%s.%0*d", logfilename,
			positions, n);
		if (n == debugnfiles) {
			unlink(from);
		} else {
			char	to[MAXPATHLEN + 1];
			snprintf(to, sizeof(to), "%s.%0*d", logfilename,
				positions, n + 1);
			rename(from, to);
		}
	}
	// close and rename current log file
	if (debug_filedescriptor >= 0) {
		close(debug_filedescriptor);
		debug_filedescriptor = -1;
		char	to[MAXPATHLEN + 1];
		snprintf(to, sizeof(to), "%s.%0*d", logfilename, positions, 0);
		rename(logfilename, to);
	}
	// reopen a new log file
	debug_file(logfilename);
}

#define	MSGSIZE	1024

static std::mutex	mtx;

typedef std::map<std::thread::id, int>	thread_map_t;

static	thread_map_t	thread_map;
static	int	nextthreadid = 1;

static int	lookupthreadid(const std::thread::id& id) {
	std::unique_lock<std::mutex>	lock(mtx);
	thread_map_t::const_iterator	i = thread_map.find(id);
	if (i != thread_map.end()) {
		return i->second;
	}
	int	newid = nextthreadid++;
	thread_map.insert(std::make_pair(id, newid));
	return newid;
}

extern "C" void vdebug(int loglevel, const char *file, int line,
	int flags, const char *format, va_list ap) {
	//time_t		t;
	struct tm	*tmp;
	char	msgbuffer[MSGSIZE], prefix[MSGSIZE],
		msgbuffer2[MSGSIZE], tstp[MSGSIZE],
		threadid[20];
	int	localerrno;

	if (loglevel > debuglevel) { return; }

	// message content
	localerrno = errno;
	vsnprintf(msgbuffer2, sizeof(msgbuffer2), format, ap);
	if (flags & DEBUG_ERRNO) {
		snprintf(msgbuffer, sizeof(msgbuffer), "%s: %s (%d)",
			msgbuffer2, strerror(localerrno), localerrno);
	} else {
		strcpy(msgbuffer, msgbuffer2);
	}

	// get time
	struct timeval	tv;
	gettimeofday(&tv, NULL);
	tmp = localtime(&tv.tv_sec);
	size_t	bytes = strftime(tstp, sizeof(tstp), "%b %e %H:%M:%S", tmp);

	// high resolution time
	if (debugtimeprecision > 0) {
		if (debugtimeprecision > 6) {
			debugtimeprecision = 6;
		}
		unsigned int	u = tv.tv_usec;
		int	p = 6 - debugtimeprecision;
		while (p--) { u /= 10; }
		snprintf(tstp + bytes, sizeof(tstp) - bytes, ".%0*u",
			debugtimeprecision, u);
	}

	// find the current thread id if necessary
	if (debugthreads) {
		snprintf(threadid, sizeof(threadid), "/%d",
			lookupthreadid(std::this_thread::get_id()));
	} else {
		threadid[0] = '\0';
	}

	// handle syslog case, where we have a much simpler 
	if (debug_destination == DEBUG_SYSLOG) {
		if (flags & DEBUG_NOFILELINE) {
			snprintf(prefix, sizeof(prefix), "%s", threadid);
		} else {
			snprintf(prefix, sizeof(prefix), "%s %s:%03d:",
				threadid, file, line);
		}
		syslog(loglevel, "%s %s", prefix, msgbuffer);
		return;
	}

	// get prefix
	if (flags & DEBUG_NOFILELINE) {
		snprintf(prefix, sizeof(prefix), "%s %s[%d%s]:",
			tstp, DEBUG_IDENT, getpid(), threadid);
	} else {
		snprintf(prefix, sizeof(prefix), "%s %s[%d%s] %s:%03d:",
			tstp, DEBUG_IDENT, getpid(), threadid, file, line);
	}

	// format log message
	if (debug_destination == DEBUG_STDERR) {
		fprintf(stderr, "%s %s\n", prefix, msgbuffer);
		fflush(stderr);
		return;
	}

	// last case is FD, where we first have to create the message
	// buffer
	snprintf(msgbuffer2, sizeof(msgbuffer2), "%s %s",
		prefix, msgbuffer);
	{
		std::unique_lock<std::mutex>	lock(mtx);
		linecounter++;
		lseek(debug_filedescriptor, 0, SEEK_END);
		write(debug_filedescriptor, msgbuffer2, strlen(msgbuffer2));
		write(debug_filedescriptor, "\n", 1);
		// check whether we have to rotate the 
		if ((debugmaxlines > 0) && (linecounter >= debugmaxlines)) {
			rotate_logfile();
		}
	}
	return;
}
