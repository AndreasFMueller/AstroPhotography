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
	// create a new image
	ImageStep	*imagestep = input();
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

/**
 * \brief Find out whether metadata exists for the image
 */
bool	ImageBufferStep::hasMetadata(const std::string& name) const {
	if (NULL == image) {
		throw std::runtime_error("no image");
	}
	return input()->hasMetadata(name);
}

/**
 * \brief Get metadata from the image
 */
astro::image::Metavalue	ImageBufferStep::getMetadata(const std::string& name) const {
	if (NULL == image) {
		throw std::runtime_error("no image");
	}
	return input()->getMetadata(name);
}

} // namespace process
} // namespace astro
