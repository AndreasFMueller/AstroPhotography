/*
 * WritableFileImageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroIO.h>
#include <mutex>

namespace astro {
namespace process {

/**
 * \brief Create a writable file image step
 */
WritableFileImageStep::WritableFileImageStep(NodePaths& parent,
	const std::string& filename)
	: FileImageStep(parent, filename) {
	_previousstate = idle;
	ProcessingStep::status(idle);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create writable file step '%s'",
		filename.c_str());
}

std::string	WritableFileImageStep::fullname() const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing full name");
	return dstname();
}

/**
 * \brief Find the status of a WritableFileImageStep
 *
 * If the file exists and the precursor and the precusor is older, then
 * we don't need to look at the precursor
 */
ProcessingStep::state	WritableFileImageStep::status() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"checking status %s, previous state is %s", dstname().c_str(),
		statename(_previousstate).c_str());
	if (precursors().size() != 1) {
		_previousstate = ProcessingStep::failed;
		return _previousstate;
	}

	// if the previous state is complete or failed, we don't need to
	// check the state
	if (_previousstate >= ProcessingStep::complete) {
		return _previousstate;
	}

	// now we know that there is exactly one precursor image
	ProcessingStepPtr	precursor
		= ProcessingStep::byid(*precursors().begin());

	// if the file already exists, then only the time matters
	if (exists()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s already exists",
			dstname().c_str());
		if (precursor->when() < when()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"precursor of '%s' is older than precursor %s: %d < %d",
				dstname().c_str(), precursor->name().c_str(),
				precursor->when(), when());
			// the precursors are older than the file, so we
			// don't need to evaluate the precursor
			_previousstate = ProcessingStep::complete;
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"precursor of %s is younger",
				dstname().c_str());
			// the precursors are younger, so the precursor
			// definitely need to be evaluated first. The state
			// therefore depends on the precursors state.
			switch (precursor->status()) {
			case idle:
			case needswork:
			case working:
				_previousstate = idle;
				break;
			case complete:
				_previousstate = needswork;
				break;
			case failed:
				_previousstate = failed;
				break;
			}
		}
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file %s does not exist",
			dstname().c_str());
		// in this case the state depends entirely on the precursor
		switch (precursor->status()) {
		case ProcessingStep::idle:
		case ProcessingStep::working:
			_previousstate = ProcessingStep::idle;
			break;
		case ProcessingStep::needswork:	
			debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor needs work");
			// in this case we have to check the age of the
			// precursor
			if (precursor->when() < when()) {
				// what the precursor has is good enough, so
				// we can work with that
				debug(LOG_DEBUG, DEBUG_LOG, 0, "we need work");
				_previousstate = ProcessingStep::needswork;
			} else {
				// what the precursor has is not good enough
				// we have to make sure it is processed first
				debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for precursor");
				_previousstate = ProcessingStep::idle;
			}
			// we should never get to this point
			break;
		case ProcessingStep::complete:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor is complete");
			_previousstate = ProcessingStep::needswork;
			break;
		case ProcessingStep::failed:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor is failed");
			_previousstate = ProcessingStep::failed;
			break;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "step '%s' is %s", name().c_str(),
		statename(_previousstate).c_str());
	return _previousstate;
}

/**
 * \brief Do the work of writing a file to disk if necessary
 */
ProcessingStep::state	WritableFileImageStep::do_work() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d start processing %s", id(),
		dstname().c_str());
	// get the predecessor image (there may only be one)
	if (precursors().size() != 1) {
		debug(LOG_ERR, DEBUG_LOG, 0, "wrong number of precursors");
		_previousstate = ProcessingStep::failed;
		return _previousstate;
	}

	// now we know that there is exactly one precursor image
	ProcessingStepPtr	precursor
		= ProcessingStep::byid(*precursors().begin());

	// we don't care about the precursor-state as long as the dependencies
	// are ok

	// if the precursor is complete, then we have an image and can
	// write 
	if (exists()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "the file '%s' already exists",
			dstname().c_str());
		if (precursor->when() < when()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "reading the file %s",
				dstname().c_str());
			_previousstate = FileImageStep::do_work();
			return _previousstate;
		}
	}

	// if the current state of the precursor is not complete, we
	// cannot use it 
	if (ProcessingStep::complete != precursor->status()) {
		_previousstate = ProcessingStep::idle;
		return _previousstate;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor found: %d", precursor->id());

	// get the image from the precursor
	ImageStep	*imagestep = dynamic_cast<ImageStep*>(&*precursor);
	if (NULL == imagestep) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"precursorstep ist no an image step: %s",
			demangle_string(*precursor).c_str());
		return ProcessingStep::failed;
	}

	_image = imagestep->image();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %s image to %s",
		_image->size().toString().c_str(), dstname().c_str());
	astro::io::FITSout	out(dstname());
	out.setPrecious(false);
	out.write(_image);

	// return complete status
	_previousstate = ProcessingStep::complete;
	return _previousstate;
}

/**
 * \brief Show what this step is going to do
 */
std::string	WritableFileImageStep::what() const {
	return stringprintf("writing FITS file %s", dstname().c_str());
}

/**
 * \brief get the image from this step
 * 
 * If the image has already been computed, we return the image, but if it
 * has not been computed, then we read it from the file.
 */
ImagePtr	WritableFileImageStep::image() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (_image) {
		return _image;
	}
	return FileImageStep::image();
}

} // namespace process
} // namespade astro
