/*
 * FileImageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <includes.h>
#include <AstroIO.h>
#include <sstream>

namespace astro {
namespace process {

/**
 * \brief Construct a file image step
 */
FileImageStep::FileImageStep(NodePaths& parent, const std::string& filename)
	: ImageStep(parent), _filename(filename) {
	_exists = false;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "node paths: %s",
		NodePaths::info().c_str());
}

/**
 * \brief Destroy a file images step
 */
FileImageStep::~FileImageStep() {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying %s", _filename.c_str());
}

std::string	FileImageStep::srcname() const {
	return srcfile(_filename);
}

std::string	FileImageStep::dstname() const {
	return dstfile(_filename);
}

std::string	FileImageStep::fullname() const {
	return srcname();
}

/**
 * \brief Get the time when a file was last changed
 *
 * This is the "when()" time of a file based image
 */
time_t	FileImageStep::when() const {
	struct stat	sb;
	std::string	_f = fullname();
	int	rc = stat(_f.c_str(), &sb);
	if (rc < 0) {
		std::string	msg = stringprintf(
			"file '%s' not accessible: %s",
			_f.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		return 0;
	}
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "find ctime of %s %d", _filename.c_str(),
//		sb.st_ctime);
	return sb.st_ctime;
}

/**
 * \brief determine the status of the step
 */
ProcessingStep::state	FileImageStep::status() {
	// check the file
	if (!exists()) {
		return ProcessingStep::failed;
	}
	return ProcessingStep::complete;
}

/**
 * \brief Get the image by reading it form disk
 */
ImagePtr	FileImageStep::image() {
	astro::io::FITSin	in(fullname());
	ImagePtr	image = in.read();
	return image;
}

/**
 * \brief Do the work, i.e. read the image from disk
 */
ProcessingStep::state	FileImageStep::do_work() {
	return status();
}

/**
 * \brief find out whether the file exists
 */
bool	FileImageStep::exists() {
	if (_exists) {
		return true;
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "check existence of '%s'",
	//	_filename.c_str());
	struct stat	sb;
	int	rc = stat(fullname().c_str(), &sb);
	if (rc < 0) {
		return false;
	}
	_exists = true;
	return true;
}

std::string	FileImageStep::what() const {
	return stringprintf("reading FITS file %s", _filename.c_str());
}

std::string	FileImageStep::verboseinfo() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "verboseinfo()");
	std::ostringstream	out;
	out << ProcessingStep::verboseinfo();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct fullname");
	out << " file=" << fullname();
	return out.str();
}

} // namespace process
} // namespace astro
