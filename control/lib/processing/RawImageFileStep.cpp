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

#if 0
//////////////////////////////////////////////////////////////////////
// Raw image in memory
//////////////////////////////////////////////////////////////////////

/**
 * \brief Constructor for in Memory image
 */
RawImageStep::RawImageStep(ImagePtr image) : _image(image) {
	// a raw image has no inputs, so it can never be in a state less 
	// than needswork
	status(ProcessingStep::needswork);
}

/**
 * \brief Access to subframe information
 */
ImageRectangle	RawImageStep::subframe() const {
	return _image->getFrame();
}

/**
 * \brief Work function of the processing step
 */
ProcessingStep::state	RawImageStep::common_work() {
	// add the preview
	_preview = PreviewAdapter::get(_image);

	// create the adapters for output
	_out = ImageStep::outPtr(new DoubleAdapter(_image));

	// if we succeed in all this, then the new state should be complete
	return ProcessingStep::complete;
}

/**
 * \brief Work for an in memory image
 */
ProcessingStep::state	RawImageStep::do_work() {
	return common_work();
}

/**
 * \brief ask whether the image has meta data
 */
bool	RawImageStep::hasMetadata(const std::string& name) const {
	return _image->hasMetadata(name);
}

/**
 * \brief get the meta data from the image
 */ 
astro::image::Metavalue	RawImageStep::getMetadata(const std::string& name) const {
	return _image->getMetadata(name);
}

ImageMetadata::const_iterator	RawImageStep::begin() const {
	return _image->begin();
}

ImageMetadata::const_iterator	RawImageStep::end() const {
	return _image->end();
}

//////////////////////////////////////////////////////////////////////
// Raw image from a file
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a processing step for raw images
 *
 * Upon initialization, the class checks whether the file exists, and sets
 * the state accordingly. 
 * \param filename	name of the raw image file
 */
RawImageFileStep::RawImageFileStep(const std::string& filename)
	: RawImageStep(ImagePtr()), _filename(filename) {
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
RawImageFileStep::~RawImageFileStep() {
}

/**
 * \brief Work function of the processing step
 */
ProcessingStep::state	RawImageFileStep::do_work() {
	// read the file
	try {
		FITSin	in(_filename);
		_image = in.read();
	} catch (std::runtime_error& x) {
		return ProcessingStep::idle;
	}

	// if we succeed in all this, then the new state should be complete
	return common_work();
}
#endif

} // namespace process
} // namespace astro
