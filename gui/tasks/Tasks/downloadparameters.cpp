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
#include <image.hh>

/**
 * \brief Construct a Downloadparameters object
 */
DownloadParameters::DownloadParameters() {
	binning = false;
	exposuretime = false;
	temperature = false;
	filter = false;
	shutter = false;
	date = false;
}

static std::string	qstring2string(const QString& qstring) {
	QByteArray ba = qstring.toLatin1();
	std::string	result(ba.data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "converted string: '%s'",
		result.c_str());
	return result;
}

/**
 * \brief Filename from task info and parameters
 */
std::string	DownloadParameters::filename(const Astro::TaskInfo_var& info,
			const Astro::TaskParameters_var& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct filename");
	std::ostringstream	out;
	std::string	d = qstring2string(directory);
	std::string	f = qstring2string(prefix);
	out << d << "/" << f;
	out << "-" << info->taskid;
	if (date) {
		char	buffer[128];
		time_t	t = time(NULL) - info->lastchange;
		struct tm	*tmp = localtime(&t);
		strftime(buffer, sizeof(buffer), "%Y%m%d-%H%M%S", tmp);
		out << "-" << buffer;
	}
	if (exposuretime) {
		int	t = round(parameters->exp.exposuretime);
		if (t <= 0) {
			t = 1;
		}
		out << "-" << t << "s";
	}
	if (temperature) {
		int	temp = round(parameters->ccdtemperature - 273.15);
		out << "-T" << temp;
	}
	if (filter) {
		out << "-F" << parameters->filterposition;
	}
	if (shutter) {
		bool	openshutter = parameters->exp.shutter
					== Astro::SHUTTER_OPEN;
		out << "-" << ((openshutter) ? "LIGHT" : "DARK");
	}
	if (binning) {
		out << "-" << parameters->exp.mode.x;
		out << "x" << parameters->exp.mode.y;
	}
	out << ".fits";
	f = out.str();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file name: %s", f.c_str());
	return out.str();
}

/**
 * \brief Download all tasks from a list of tasks
 */
void	DownloadParameters::download(Astro::TaskQueue_var taskqueue,
		const std::list<long>& taskids) {
	std::list<long>::const_iterator	i;
	for (i = taskids.begin(); i != taskids.end(); i++) {
		int	taskid = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "downloading taskid %d", taskid);
		download(taskqueue, taskid);
	}
}

/**
 * \brief Download a task based on the task id
 */
void	DownloadParameters::download(Astro::TaskQueue_var taskqueue,
		long taskid) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "taskid = %d", taskid);
	Astro::TaskInfo_var	info = taskqueue->info(taskid);
	Astro::TaskParameters_var	parameters
		= taskqueue->parameters(taskid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parameters received");
	std::string	f = filename(info, parameters);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", f.c_str());

	// create a file
	int	fd = open(f.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd < 0) {
		throw std::runtime_error("cannot create new file");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s created", f.c_str());

	try {
		// get the task and the image from the server
		Astro::Task_var	task = taskqueue->getTask(taskid);
		Astro::Image_var	image = task->getImage();

		// get the file data
		Astro::Image::ImageFile_var	imagefile = image->file();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file has %d bytes",
			imagefile->length());

		// write the file data
		write(fd, imagefile->get_buffer(), imagefile->length());
		close(fd);
	} catch (...) {
		close(fd);
	}

}

/**
 * \brief whether any attribute is to be included in the file names
 */
bool	DownloadParameters::usetaskid() const {
	return !(exposuretime || binning || filter || shutter || temperature || date);
}
