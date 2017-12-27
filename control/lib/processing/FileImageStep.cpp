/*
 * FileImageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <includes.h>
#include <AstroIO.h>

namespace astro {
namespace process {

/**
 * \brief Construct a file image step
 */
FileImageStep::FileImageStep(const std::string& filename)
	: _filename(filename) {
	_exists = false;
}

/**
 * \brief Destroy a file images step
 */
FileImageStep::~FileImageStep() {
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying %s", _filename.c_str());
}

/**
 * \brief Get the time when a file was last changed
 *
 * This is the "when()" time of a file based image
 */
time_t	FileImageStep::when() const {
	struct stat	sb;
	int	rc = stat(_filename.c_str(), &sb);
	if (rc < 0) {
		std::string	msg = stringprintf(
			"file '%s' not accessible: %s",
			_filename.c_str(), strerror(errno));
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
#if 0
	// if we are presently working, return the working state
	if (_status == ProcessingStep::working) {
		return _status;
	}
#endif

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
	astro::io::FITSin	in(_filename);
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
	int	rc = stat(_filename.c_str(), &sb);
	if (rc < 0) {
		return false;
	}
	_exists = true;
	return true;
}

std::string	FileImageStep::what() const {
	return stringprintf("reading FITS file %s", _filename.c_str());
}

} // namespace process
} // namespace astro
