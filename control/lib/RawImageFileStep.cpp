/*
 * RawImageFileStep.cpp -- Processing Step that reads a raw image and provides
 *                         preview access 
 *
 * (c) Prof Dr Andreas Mueller,  Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <includes.h>

using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace process {

/**
 * \brief Create a processing step for raw images
 *
 * Upon initialization, the class checks whether the file exists, and sets
 * the state accordingly. 
 * \param filename	name of the raw image file
 */
RawImageFile::RawImageFile(const std::string& filename) : _filename(filename) {
	// initialize undefined fields
	_out = NULL;

	// test whether the file exists
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		status(ProcessingStep::idle);
		return;
	}
	// test whether the file is readable
	if (access(filename.c_str(), R_OK) < 0) {
		status(ProcessingStep::idle);
		return;
	}
	status(ProcessingStep::needswork);
}

/**
 * \brief Destroy the step
 */
RawImageFile::~RawImageFile() {
}

/**
 * \brief Work function of the processing step
 */
ProcessingStep::state	RawImageFile::do_work() {
	// read the file
	try {
		FITSin	in(_filename);
		_image = in.read();
	} catch (std::runtime_error& x) {
		return ProcessingStep::idle;
	}

	// add the preview
	preview = PreviewAdapter::get(_image);

	// create the adapters for output
	_out = ProcessingStep::outPtr(new DoubleAdapter(_image));

	// if we succeed in all this, then the new state should be complete
	return ProcessingStep::complete;
}

} // namespace process
} // namespace astro
