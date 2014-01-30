/*
 * downloadparameters.cpp -- download parameters and methods
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <downloadparameters.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <sstream>
#include <sys/time.h>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <errno.h>

/**
 * \brief Construct a Downloadparameters object
 *
 * The default is not to 
 */
DownloadParameters::DownloadParameters() {
	binning = false;
	exposuretime = false;
	temperature = false;
	filter = false;
	shutter = false;
	date = false;
}

/**
 * \brief Auxiliary string conversion functions
 *
 * Convert a QString to a STL string.
 */
static std::string	qstring2string(const QString& qstring) {
	QByteArray ba = qstring.toLatin1();
	std::string	result(ba.data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted string: '%s'",
		result.c_str());
	return result;
}

/**
 * \brief Filename from task info and parameters
 *
 * \param info		Information about the task
 * \param parameters	Parameters for a task
 * \return		The name of the file
 */
std::string	DownloadParameters::filename(const Astro::TaskInfo_var& info,
			const Astro::TaskParameters_var& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct filename");
	std::ostringstream	out;
	std::string	d = qstring2string(directory);
	std::string	f = qstring2string(prefix);
	out << d << "/" << f;
	out << "-" << info->taskid;

	// date component
	if (date) {
		char	buffer[128];
		time_t	t = time(NULL) - info->lastchange;
		struct tm	*tmp = localtime(&t);
		strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", tmp);
		out << "-" << buffer;
	}

	// include ht exposure time in the file name
	if (exposuretime) {
		int	t = round(parameters->exp.exposuretime);
		if (t <= 0) {
			t = 1;
		}
		out << "-" << t << "s";
	}

	// include the chip temperature in the file name
	if (temperature) {
		int	temp = round(parameters->ccdtemperature - 273.15);
		out << "-T" << temp;
	}

	// include the filename 
	if ((filter) && (strlen(parameters->filterwheel.in()) > 0)) {
		out << "-F" << parameters->filterposition;
	}

	// depending on the shutter, include the string LIGHT/DARK
	if (shutter) {
		bool	openshutter = parameters->exp.shutter
					== Astro::SHUTTER_OPEN;
		out << "-" << ((openshutter) ? "LIGHT" : "DARK");
	}

	// add the binning mode to the filename
	if (binning) {
		out << "-" << parameters->exp.mode.x;
		out << "x" << parameters->exp.mode.y;
	}

	// append the FITS extension
	out << ".fits";

	// that's it, convert to the filename
	f = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file name: %s", f.c_str());
	return f;
}

/**
 * \brief Download all tasks from a list of tasks
 */
std::list<fileinfo>	DownloadParameters::download(Astro::TaskQueue_var& taskqueue,
		const std::list<long>& taskids) {
	// prepare the empty result list
	std::list<fileinfo>	result;

	// iterate through the selection
	std::list<long>::const_iterator	i;
	for (i = taskids.begin(); i != taskids.end(); i++) {
		int	taskid = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "downloading taskid %d", taskid);
		// download the file. Since it may have been deleted, or is
		// not in the right state, exceptions indicating this problem
		// can occur, which we simply ignore
		try {
			fileinfo	fi = download(taskqueue, taskid);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"%d bytes downloaded to %s", fi.size,
				fi.name.c_str());
			result.push_back(fi);
		} catch (Astro::NotFound& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d does not exist",
				taskid);
		} catch (const std::runtime_error& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"task %d cannot be downloaded: %s", x.what());
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown exception");
		}
	}
	return result;
}

/**
 * \brief Download a task based on the task id
 *
 * This method downloads the file associated with a given task.
 * \param taskqueue	The TaskQueue reference to use to talk to the server
 * \param taskid	The task number
 * \return		A class containing file name used and file size
 */
fileinfo	DownloadParameters::download(Astro::TaskQueue_var& taskqueue,
		long taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskid = %d", taskid);

	// get information about the task. We don't want to rely on the
	// information in the taskmainwindow, because that could be outdated.
	// Tasks my have udpated since the download began, and because all
	// downloading as well as status udpates are performed on the main
	// thread, status udpates are blocked until the downloads complete.
	Astro::TaskInfo_var	info = taskqueue->info(taskid);

	// the task does not have the right state, we can as well give
	// up here. Only completed tasks can be downloaded
	if (info->state != Astro::TASK_COMPLETED) {
		debug(LOG_ERR, DEBUG_LOG, 0, "task %d not completed", taskid);
		std::string	msg = astro::stringprintf(
					"task %d not completed", taskid);
		throw std::runtime_error(msg);
	}

	// for the filenames, also the task parameters are needed
	Astro::TaskParameters_var	parameters
		= taskqueue->parameters(taskid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parameters received");
	fileinfo	file(filename(info, parameters));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", file.name.c_str());

	// create a file
	int	fd = open(file.name.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
			0666);
	if (fd < 0) {
		throw std::runtime_error("cannot create new file");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s created", file.name.c_str());

	try {
		// get the task and the image from the server
		Astro::Task_var	task = taskqueue->getTask(taskid);
		Astro::Image_var	image = task->getImage();

		// get the file data
		Astro::Image::ImageFile_var	imagefile = image->file();
		file.size = imagefile->length();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file has %d bytes",
			file.size);

		// write the file data
		if (write(fd, imagefile->get_buffer(), file.size)
			!= file.size) {
			std::string	msg = astro::stringprintf(
				"cannot write file: %s", strerror(errno));
			throw std::runtime_error(msg);
		}
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error during download of task %s",
			taskid);
		close(fd);
		unlink(file.name.c_str());	// remove the spurious file
	}
	close(fd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download of %d to %s complete",
		taskid, file.name.c_str());
	return file;
}

/**
 * \brief output the downloadparameters to a stream
 */
std::ostream&	operator<<(std::ostream& out, const DownloadParameters& p) {
	out << "dir=" << qstring2string(p.directory) << " ";
	out << "prefix=" << qstring2string(p.prefix) << " ";
	out << "exposuretime=" << ((p.exposuretime) ? "YES" : "NO");
	out << "binning=" << ((p.binning) ? "YES" : "NO");
	out << "shutter=" << ((p.shutter) ? "YES" : "NO");
	out << "filter=" << ((p.filter) ? "YES" : "NO");
	out << "temperature=" << ((p.temperature) ? "YES" : "NO");
	out << "date=" << ((p.date) ? "YES" : "NO");
	return out;
}

/**
 * \brief convert the downloadparameters to a string
 */
std::string	DownloadParameters::toString() const {
	std::ostringstream	out;
	out << *this;
	return out.str();
}
