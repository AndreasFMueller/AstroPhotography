/*
 * DaemonI.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "DaemonI.h"
#include <IceConversions.h>
#include <CommunicatorSingleton.h>
#include <thread>
#include <sys/stat.h>
#include "Restart.h"
#include <AstroFormat.h>
#include <dirent.h>
#include <unistd.h>
#include <version.h>
#include <AstroUtils.h>
#include <sys/utsname.h>
#include <sys/times.h>
#include <sys/sysctl.h>
#ifdef __linux__
#include <sys/sysinfo.h>
#endif /* __linux__ */

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#endif

namespace snowstar {

static void     do_shutdown(float delay, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutting down communicator in %f",
		delay);
	useconds_t	udelay = 1000000 * delay;
	usleep(udelay);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutting down communicator now");
	current.adapter->getCommunicator()->shutdown();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown complete");
}

/**
 * \brief Reload the repository database
 */
void	DaemonI::reloadRepositories(const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repositories reloaded");
	_server.reloadRepositories();
}

/**
 * \brief Initiate shutdown of the server process
 */
void    DaemonI::shutdownServer(Ice::Float delay,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server shutdown requested");
	Restart::shutdown_instead(true);
	std::thread	t(do_shutdown, delay, current);
	t.detach();
	try {
		Heartbeat::terminate(true);
	} catch (const std::exception& x) {
	}
}

static void	do_shutdownsystem(float delay, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutting down systen in %f", delay);
	useconds_t	udelay = 1000000 * delay;
	usleep(udelay);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutting down system now");
	system("sudo shutdown -h now");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown command sent");
}

/**
 * \brief Initiate shutdown of the system
 */
void	DaemonI::shutdownSystem(Ice::Float delay, const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "shutdown request");
	std::thread	t(do_shutdownsystem, delay, current);
	t.detach();
}

/**
 * \brief Initiate restart of the server
 */
void    DaemonI::restartServer(Ice::Float delay,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server restart requested");
	Restart::shutdown_instead(false);
	std::thread	t(do_shutdown, delay, current);
	t.detach();
}

/**
 * \brief Get information about the directory
 */
DirectoryInfo	DaemonI::statDirectory(const std::string& dirname,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statDirectory(%s)", dirname.c_str());
	struct stat	sb;
	if (stat(dirname.c_str(), &sb) < 0) {
		if (ENOENT == errno) {
			NotFound	notfound;
			notfound.cause = astro::stringprintf("directory %s "
				"not found", dirname.c_str());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s",
				notfound.cause.c_str());
			throw  notfound;
		}
	}
	if (!S_ISDIR(sb.st_mode)) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("%s is not a directory",
			dirname.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s",
			notfound.cause.c_str());
		throw notfound;
	}

	DirectoryInfo	info;
	info.name = dirname;
	info.writeable = (0 == access(dirname.c_str(), W_OK)) ? true : false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "directory '%s' is %swritable",
		dirname.c_str(), (info.writeable) ? "" : "not ");

	// get all file names
	DIR	*dirp = opendir(dirname.c_str());
	if (NULL == dirp) {
		std::string	msg = astro::stringprintf("cannot open "
			"directory %s: %s", dirname.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		IOException	error;
		error.cause = msg;
		throw error;
	}
	struct dirent	direntry;
	struct dirent	*direntryp;
	do {
		int	rc = readdir_r(dirp, &direntry, &direntryp);
		if (rc) {
			std::string	msg = astro::stringprintf("can't read "
				"dir: %s", strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			IOException	error;
			error.cause = msg;
			closedir(dirp);
			throw error;
		}
		if (NULL == direntryp)
			continue;
		std::string	entryname(direntry.d_name);
		if (direntry.d_type == DT_REG) {
			info.files.push_back(entryname);
		}
		if (direntry.d_type == DT_DIR) {
			info.directories.push_back(entryname);
		}
	} while (NULL != direntryp);
	closedir(dirp);

	return info;
}

/**
 * \brief Get information about a file
 */
FileInfo	DaemonI::statFile(const std::string& filename,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statFile(%s)", filename.c_str());
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("cannot stat %s: %s",
			filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
	if (!S_ISREG(sb.st_mode)) {
		IOException	notfile;
		notfile.cause = astro::stringprintf("%s not a file",
			filename.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", notfile.cause.c_str());
		throw notfile;
	}
	FileInfo	fi;
	fi.name = filename;
	fi.writeable = (0 == access(filename.c_str(), W_OK)) ? true : false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s is %swritable", 
		filename.c_str(), (fi.writeable) ? "" : "not ");
	return fi;
}

/**
 * \brief Get information about a block device
 */
FileInfo	DaemonI::statDevice(const std::string& devicename,
			const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "statDevice(%s)", devicename.c_str());
	struct stat	sb;
	if (stat(devicename.c_str(), &sb) < 0) {
		NotFound	notfound;
		notfound.cause = astro::stringprintf("cannot stat %s: %s",
			devicename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", notfound.cause.c_str());
		throw notfound;
	}
	if (!S_ISBLK(sb.st_mode)) {
		IOException	notfile;
		notfile.cause = astro::stringprintf("%s not a device",
			devicename.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", notfile.cause.c_str());
		throw notfile;
	}
	FileInfo	fi;
	fi.name = devicename;
	fi.writeable = (0 == access(devicename.c_str(), W_OK)) ? true : false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device %s is %swritable", 
		devicename.c_str(), (fi.writeable) ? "" : "not ");
	return fi;
}

/**
 * \brief Mount a device
 */
void	DaemonI::mount(const std::string& device, const std::string& mountpoint,
		const Ice::Current& current) {
	CallStatistics::count(current);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mounting %s on %s", device.c_str(),
		mountpoint.c_str());

	// first check that the device exists
	struct stat	sb;
	if (stat(device.c_str(), &sb) < 0) {
		NotFound	error;
		error.cause = astro::stringprintf("cannot stat '%s': %s",
			device.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}
	if (!S_ISBLK(sb.st_mode)) {
		IOException	error;
		error.cause = astro::stringprintf("%s is not a block device",
				device.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}

	// check that the mount point exists
	if (stat(mountpoint.c_str(), &sb) < 0) {
		NotFound	error;
		error.cause = astro::stringprintf("cannot stat '%s': %s",
			mountpoint.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}
	if (!S_ISDIR(sb.st_mode)) {
		IOException	error;
		error.cause = astro::stringprintf("%s is not a directory",
				mountpoint.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}

	// perform the mount command
	// XXX we should do this via popen, to be able to read the error stream
	std::string	command = astro::stringprintf("%s -t vfat %s %s",
		MOUNT_COMMAND, device.c_str(), mountpoint.c_str());
	int	rc = system(command.c_str());
	if (rc) {
		OperationFailed	error;
		error.cause = astro::stringprintf("cannot mount %s on %s: %s",
			device.c_str(), mountpoint.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}
}

/**
 * \brief unmount a directory
 */
void	DaemonI::unmount(const std::string& mountpoint,
		const Ice::Current& current) {
	CallStatistics::count(current);
	struct stat	sb;
	if (stat(mountpoint.c_str(), &sb) < 0) {
		NotFound	error;
		error.cause = astro::stringprintf("cannot stat %s: %s",
			mountpoint.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}
	if (!S_ISDIR(sb.st_mode)) {
	}
	std::string	cmd = astro::stringprintf("%s %s", UMOUNT_COMMAND,
				mountpoint.c_str());
	int	rc = system(cmd.c_str());
	if (rc) {
		IOException	error;
		error.cause = astro::stringprintf("cannot unmount %s: %s",
			mountpoint.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", error.cause.c_str());
		throw error;
	}
}

/**
 * \brief get the system time
 */
Ice::Long	DaemonI::getSystemTime(const Ice::Current& current) {
	CallStatistics::count(current);
	time_t	now;
	time(&now);
	return now;
}

/**
 * \brief set the system time
 *
 * \param unixtime	the unix time to set
 */
void	DaemonI::setSystemTime(Ice::Long unixtime,
		const Ice::Current& current) {
	CallStatistics::count(current);
	time_t	t = unixtime;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting system time to %s", ctime(&t));


	// construct the command
	struct tm	*tp = localtime(&t);
	char	buffer[30];
	std::string	cmd;
#if BSD_DATE == 1
	// BSD date command
	strftime(buffer, sizeof(buffer), "%m%d%H%M%Y.%S", tp);
	cmd = astro::stringprintf("sudo date %s", buffer);
#else /* BSD_DATE == 1 */
	// GNU date command
	// note that on Ubuntu this only works if ntp is disabled using
	// 'timedatectl set-ntp false'
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tp);
	cmd = astro::stringprintf("sudo date --set='%s'", buffer);
#endif /* BSD_DATE == 1 */
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time set command: %s", cmd.c_str());

	// execute the command
	int	rc = system(cmd.c_str());
	if (rc) {
		// throw an exception
		OperationFailed	f;
		f.cause = std::string(strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting time failed: %s",
			f.cause.c_str());
		throw f;
	}
}

std::string	DaemonI::osVersion(const Ice::Current& current) {
	CallStatistics::count(current);
	struct utsname	u;
	uname(&u);
	return std::string(u.version);
}

std::string	DaemonI::astroVersion(const Ice::Current& current) {
	CallStatistics::count(current);
	return astro::version();
}

std::string	DaemonI::snowstarVersion(const Ice::Current& current) {
	CallStatistics::count(current);
	return astro::stringprintf("%s - %s %s", snowstar::version.c_str(),
		__DATE__ ,__TIME__);
}

Sysinfo	DaemonI::getSysinfo(const Ice::Current& current) {
	CallStatistics::count(current);
	Sysinfo	result;
	result.uptime = 0;
	result.load1min = 0;
	result.load5min = 0;
	result.load15min = 0;
	result.totalram = 0;
	result.sharedram = 0;
	result.bufferram = 0;
	result.totalswap = 0;
	result.freeswap = 0;
	result.processes = 0;
#if __APPLE__
#if 0
	NotImplemented	notimplemented;
	notimplemented.cause = std::string("sysinfo not available");
	throw notimplemented;
#else
	// XXX get memory size
	char	buffer[1024];
	size_t	s = sizeof(buffer);
	if (0 == sysctlbyname("hw.memsize", buffer, &s, 0, 0)) {
		result.totalram = (*(int64_t *)buffer);
	}
#endif
#endif
#if __linux__
	struct sysinfo	info;
	if (sysinfo(&info) < 0) {
		std::string	msg = astro::stringprintf("no sysinfo: %s",
			strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		NotImplemented	notimplemented;
		notimplemented.cause = msg;
		throw notimplemented;
	}
	result.uptime = info.uptime;
	result.load1min = info.loads[0] / 65535.;
	result.load5min = info.loads[1] / 65535.;
	result.load15min = info.loads[2] / 65535.;
	result.totalram = info.totalram * info.mem_unit;
	result.freeram = info.freeram * info.mem_unit;
	result.sharedram = info.sharedram * info.mem_unit;
	result.bufferram = info.bufferram * info.mem_unit;
	result.totalswap = info.totalswap * info.mem_unit;
	result.freeswap = info.freeswap * info.mem_unit;
	result.processes = info.procs;
#endif
	return result;
}

float	DaemonI::daemonUptime(const Ice::Current& current) {
	CallStatistics::count(current);
	long	ticks = sysconf(_SC_CLK_TCK);
	struct tms	t;
	return (times(&t) - _server.start_clock()) / (float)ticks;
}

float	DaemonI::cputime(const Ice::Current& current) {
	CallStatistics::count(current);
	long	ticks = sysconf(_SC_CLK_TCK);
	struct tms	t;
	times(&t);
	return (t.tms_utime + t.tms_stime) / (float)ticks;
}

/**
 * Returns the current resident set size (physical memory use) measured
 * in bytes, or zero if the value cannot be determined on this OS.
 * (found on https://stackoverflow.com/questions/669438/how-to-get-memory-usage-at-runtime-using-c)
 */
static size_t getCurrentRSS( ) {
#if defined(__APPLE__) && defined(__MACH__)
    /* OSX ------------------------------------------------------ */
    struct mach_task_basic_info info;
    mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self( ), MACH_TASK_BASIC_INFO,
        (task_info_t)&info, &infoCount ) != KERN_SUCCESS )
        return (size_t)0L;      /* Can't access? */
    return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
    /* Linux ---------------------------------------------------- */
    long rss = 0L;
    FILE* fp = NULL;
    if ( (fp = fopen( "/proc/self/statm", "r" )) == NULL )
        return (size_t)0L;      /* Can't open? */
    if ( fscanf( fp, "%*s%ld", &rss ) != 1 )
    {
        fclose( fp );
        return (size_t)0L;      /* Can't read? */
    }
    fclose( fp );
    return (size_t)rss * (size_t)sysconf( _SC_PAGESIZE);

#else
    /* AIX, BSD, Solaris, and Unknown OS ------------------------ */
    return (size_t)0L;          /* Unsupported. */
#endif
}

float	DaemonI::processSize(const Ice::Current& current) {
	CallStatistics::count(current);
	return getCurrentRSS();
}

/**
 * \brief Retrieve the core temperature
 */
float	DaemonI::getTemperature(const Ice::Current& current) {
	CallStatistics::count(current);
	try {
		astro::Temperature	t = astro::Temperature::core();
		return t.temperature();
	} catch (const std::exception& x) {
		NotImplemented	notimplemented;
		notimplemented.cause = std::string(x.what());
		throw notimplemented;
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get temperature");
	}
	return astro::Temperature::zero;
}

/**
 * \brief Register a heartbeat monitor
 *
 * \param heartbeatmonitor	the monitor to register
 * \param current		the current call context
 */
void	DaemonI::registerHeartbeatMonitor(const Ice::Identity& heartbeatmonitor,
		const Ice::Current& current) {
	CallStatistics::count(current);
	Heartbeat::doregister(heartbeatmonitor, current);
}

/**
 * \brief Unregister a heartbeat monitor
 *
 * \param heartbeatmonitor	the monitor to unregister
 * \param current		the current call context
 */
void	DaemonI::unregisterHeartbeatMonitor(
		const Ice::Identity& heartbeatmonitor,
		const Ice::Current& current) {
	CallStatistics::count(current);
	Heartbeat::unregister(heartbeatmonitor, current);
}

/**
 * \brief Get the heartbeat interval
 *
 * \param current	the current call context
 */
Ice::Float	DaemonI::heartbeatInterval(const Ice::Current& current) {
	CallStatistics::count(current);
	return Heartbeat::interval();
}

/**
 * \brief Chagen the heartbeat interval
 *
 * \param interval	the new heartbeat interval
 * \param current	the current call context
 */
void	DaemonI::setHeartbeatInterval(Ice::Float interval,
		const Ice::Current& current) {
	CallStatistics::count(current);
	Heartbeat::interval(interval);
}

/**
 * \brief Pause the heartbeat
 *
 * \param current	current call context
 */
void	DaemonI::pauseHeartbeat(const Ice::Current& current) {
	CallStatistics::count(current);
	Heartbeat::pause();
}

/**
 * \brief Resume the heartbeat
 *
 * \param current	current call context
 */
void	DaemonI::resumeHeartbeat(const Ice::Current& current) {
	CallStatistics::count(current);
	Heartbeat::resume();
}

/**
 * \brief Check whether the heartbeat is paused
 *
 * \param current	current call context
 */
bool	DaemonI::heartbeatPaused(const Ice::Current& current) {
	CallStatistics::count(current);
	return Heartbeat::paused();
}

} // namespace snowstar
