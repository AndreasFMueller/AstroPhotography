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
	_lastread = std::numeric_limits<time_t>::max();
}

/**
 * \brief Destroy a file images step
 */
FileImageStep::~FileImageStep() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying %s", _filename.c_str());
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
ProcessingStep::state	FileImageStep::status() const {
	// if we are presently working, return the working state
	if (_status == ProcessingStep::working) {
		return _status;
	}

	// check the file
	struct stat	sb;
	int	rc = stat(_filename.c_str(), &sb);
	if (rc < 0) {
		// if there is no file, we have failed
		return ProcessingStep::failed;
	}

	// if the file we have in memory is stale, we need work
	if (_image) {
		if (sb.st_ctime > _lastread) {
			return ProcessingStep::needswork;
		} else {
			return ProcessingStep::complete;
		}
	} else {
		// we have no image yet, so reading it is required
		return ProcessingStep::needswork;
	}

	// we should never get to this point, so we return the failed state
	return ProcessingStep::failed;
}

/**
 * \brief Get the image by reading it form disk
 */
ImagePtr	FileImageStep::image() {
	if (_image) {
		return _image;
	}
	astro::io::FITSin	in(_filename);
	_image = in.read();
	time(&_lastread);
	return _image;
}

/**
 * \brief Do the work, i.e. read the image from disk
 */
ProcessingStep::state	FileImageStep::do_work() {
	if (_image) {
		if (when() < _lastread) {
			return ProcessingStep::complete;
		}
	}
	astro::io::FITSin	in(_filename);
	_image = in.read();
	return ProcessingStep::complete;
}

/**
 * \brief find out whether the file exists
 */
bool	FileImageStep::exists() const {
	struct stat	sb;
	int	rc = stat(_filename.c_str(), &sb);
	if (rc < 0) {
		return false;
	}
	return true;
}

std::string	FileImageStep::what() const {
	return stringprintf("reading FITS file %s", _filename.c_str());
}

} // namespace process
} // namespace astro
