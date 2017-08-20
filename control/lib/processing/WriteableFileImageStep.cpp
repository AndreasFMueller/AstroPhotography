/*
 * WriteableFileImageStep.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroIO.h>

namespace astro {
namespace process {

/**
 * \brief Create a writable file image step
 */
WriteableFileImageStep::WriteableFileImageStep(const std::string& filename)
	: FileImageStep(filename) {
}

/**
 * \brief Do the work of writing a file to disk if necessary
 */
ProcessingStep::state	WriteableFileImageStep::do_work() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d start processing %s", id(),
		_filename.c_str());
	// get the predecessor image (there may only be one)
	if (precursors().size() != 1) {
		return ProcessingStep::failed;
	}

	// now we know that there is exactly one precursor image
	ProcessingStepPtr	precursor
		= ProcessingStep::byid(*precursors().begin());

	// if the current state of the precursor is not complete, we
	// cannot use it 
	if (ProcessingStep::complete != precursor->status()) {
		return precursor->status();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "precursor found: %d", precursor->id());

	// if the precursor is complete, then we have an image and can
	// write 
	if (exists()) {
		if (precursor->when() < when()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "reading the file %s",
				_filename.c_str());
			return FileImageStep::do_work();
		}
	}

	// get the image from the precursor
	ImageStep	*imagestep = dynamic_cast<ImageStep*>(&*precursor);
	if (NULL == imagestep) {
		return ProcessingStep::failed;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing image to %s",
		_filename.c_str());
	_image = imagestep->image();
	astro::io::FITSout	out(_filename);
	out.setPrecious(false);
	out.write(_image);

	// set the timestamp to the current time
	time_t	now;
	time(&now);
	ProcessingStep::when(now);

	// return complete status
	return ProcessingStep::complete;
}

} // namespace process
} // namespade astro
