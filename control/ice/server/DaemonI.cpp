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

namespace snowstar {

static void     do_shutdown(float delay, const Ice::Current& current) {
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
void	DaemonI::reloadRepositories(const Ice::Current& /* current */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repositories reloaded");
	_server.reloadRepositories();
}

/**
 * \brief Initiate shutdown of the server
 */
void    DaemonI::shutdownServer(Ice::Float delay,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server shutdown requested");
	Restart::shutdown_instead(true);
	std::thread	*t = new std::thread(do_shutdown, delay, current);
}

/**
 * \brief Initiate restart of the server
 */
void    DaemonI::restartServer(Ice::Float delay,
		const Ice::Current& current) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "server restart requested");
	Restart::shutdown_instead(false);
	std::thread	*t = new std::thread(do_shutdown, delay, current);
}

/**
 * \brief Get information about the directory
 */
DirectoryInfo	DaemonI::statDirectory(const std::string& dirname,
			const Ice::Current& /* current */) {
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
			const Ice::Current& /* current */) {
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
			const Ice::Current& /* current */) {
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
		const Ice::Current& /* current */) {
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
		const Ice::Current& /* current */) {
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
Ice::Long	DaemonI::getSystemTime(const Ice::Current& /* current */) {
	time_t	now;
	time(&now);
	return now;
}

/**
 * \brief set the system time
 */
void	DaemonI::setSystemTime(Ice::Long unixtime,
		const Ice::Current& /* current */) {
	time_t	t = unixtime;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting system time to %s", ctime(&t));
	// construct the command
	struct tm	*tp = localtime(&t);
	char	buffer[30];
	strftime(buffer, sizeof(buffer), "%m%d%H%M%Y", tp);
	std::string	cmd = astro::stringprintf("sudo date %s", buffer);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time set command: %s", cmd.c_str());

	// execute the command
	int	rc = system(cmd.c_str());
	if (rc) {
		// throw an exception
		OperationFailed	f;
		f.cause = std::string(strerror(errno));
		throw f;
	}
}

} // namespace snowstar
