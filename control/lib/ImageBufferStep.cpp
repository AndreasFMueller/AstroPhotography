/*
 * ImageBufferStep.cpp -- buffer an image in memory
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Constructor
 */
ImageBufferStep::ImageBufferStep() {
	image = NULL;
}

/**
 * \brief The processing step creates a buffered image
 */
ProcessingStep::state	ImageBufferStep::do_work() {
	// get the first of the image inputs
	steps::const_iterator	i
		= std::find_if(precursors().begin(), precursors().end(),
			[](ProcessingStep *step) {
				return (NULL != dynamic_cast<ImageStep *>(step));
			}
	);

	// check whether we have a precursor image
	if (i == precursors().end()) {
		throw std::runtime_error("no precursor image");
	}

	// create a new image
	ImageStep	*imagestep = dynamic_cast<ImageStep *>(*i);
	image = new Image<double>(imagestep->out());
	imageptr = ImagePtr(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created %s image buffer",
		image->size().toString().c_str());

	// create the preview
	_preview = PreviewAdapter::get(imageptr);

	// buffer created
	return ProcessingStep::complete;
}

/**
 * \brief Get the output reference
 */
const ConstImageAdapter<double>&	ImageBufferStep::out() const {
	if (NULL == image) {
		throw std::runtime_error("no image present");
	}
	return *image;
}

} // namespace process
} // namespace astro
