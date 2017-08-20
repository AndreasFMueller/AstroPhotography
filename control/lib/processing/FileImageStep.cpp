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
	when();
}

/**
 * \brief Get the time when a file was last changed
 *
 * This is the "when()" time of a file based image
 */
time_t	FileImageStep::when() {
	struct stat	sb;
	int	rc = stat(_filename.c_str(), &sb);
	if (rc < 0) {
		std::string	msg = stringprintf(
			"file '%s' not accessible: %s",
			_filename.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		ProcessingStep::when(std::numeric_limits<time_t>::max());
	}
	ProcessingStep::when(sb.st_ctime);
	ProcessingStep::status(ProcessingStep::needswork);
	return ProcessingStep::when();
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
	return _image;
}

/**
 * \brief Do the work, i.e. read the image from disk
 */
ProcessingStep::state	FileImageStep::do_work() {
	astro::io::FITSin	in(_filename);
	_image = in.read();
	return ProcessingStep::complete;
}

/**
 * \brief find out whether the file exists
 */
bool	FileImageStep::exists() {
	return (std::numeric_limits<time_t>::max() == when());
}

} // namespace process
} // namespace astro
