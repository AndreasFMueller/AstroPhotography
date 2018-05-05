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
			const Ice::Current& current) {
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
		std::string	entryname(direntry.d_name, direntry.d_namlen);
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
 * \brief Mount a device
 */
void	DaemonI::mount(const std::string& device, const std::string& mountpoint,
		const Ice::Current& current) {
	// XXX implementation needed
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mounting %s on %s", device.c_str(),
		mountpoint.c_str());
}

} // namespace snowstar
