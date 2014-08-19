/*
 * CalibrationProcessorStep.cpp -- implementation of creators for calibration
 *                                 images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>

using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Create a new calibration processor
 */
CalibrationProcessor::CalibrationProcessor(CalibrationImage::caltype t)
	: CalibrationImage(t) {
	rawimages = NULL;
	nrawimages = 0;
}

/**
 * \brief Destroy the calibration processor
 */
CalibrationProcessor::~CalibrationProcessor() {
	if (NULL != rawimages) {
		delete rawimages;
	}
}

/**
 * \brief Find all RawImage precursors
 *
 * get pointers to all the precursors of type ImageStep, others don't have
 * image data output, so they cannot be used to build calibration images
 */
size_t	CalibrationProcessor::getPrecursors() {
	typedef ImageStep *ImageStepPtr;
	rawimages = new ImageStepPtr[precursors().size()];
	nrawimages = 0;

	// the lambda below cannot capture members, so we have to capture
	// local variables
	ImageStep	**ri = rawimages;
	int	n = 0;
	std::for_each(precursors().begin(), precursors().end(),
		[ri,n](ProcessingStep *step) mutable {
			ImageStep	*f = dynamic_cast<ImageStep *>(step);
			if (NULL != f) {
				ri[n++] = f;
			}
		}
	);
	nrawimages = n;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d raw images");
	return nrawimages;
}

/**
 * \brief Common work for both ClabrationProcessors
 *
 * This step essentially takes care of getting all the precursor images
 */
ProcessingStep::state	CalibrationProcessor::common_work() {
	if (NULL != rawimages) {
		delete rawimages;
		rawimages = NULL;
		nrawimages = 0;
	}
	getPrecursors();
	if (nrawimages == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no raw images found");
		return ProcessingStep::idle;
	}

	// ensure all images have the same size
	ImageSize	size = rawimages[0]->out().getSize();
	for (size_t i = 1; i < nrawimages; i++) {
		ImageSize	newsize = rawimages[i]->out().getSize();
		if (newsize != size) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"image %d differs in size: %s != %s",
				newsize.toString().c_str(),
				size.toString().c_str());
			return ProcessingStep::idle;
		}
	}

	// now build a targemt image
	if (nrawimages == 0) {
		throw std::runtime_error("no template size");
	}
	Image<double>	*_image
		= new Image<double>(rawimages[0]->out().getSize());
	imageptr = ImagePtr(_image);
	image = _image;
	image->fill(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create empty %s image",
		image->size().toString().c_str());

	// make the image availabe as preview
	_preview = PreviewAdapter::get(imageptr);

	// common work done
	return ProcessingStep::complete;
}

/**
 * \brief get pixel values at a given point
 */
void	CalibrationProcessor::get(unsigned int x, unsigned int y,
	double *values, int& n) {
	n = 0;
	for (size_t i = 0; i < nrawimages; i++) {
		double	v = rawimages[i]->out().pixel(x, y);
		if (v == v) {
			values[n++] = v;
		}
	}
}

/**
 * \brief access to the calibration image
 */
const ConstImageAdapter<double>&	CalibrationProcessor::out() const {
	if (NULL == image) {
		throw std::runtime_error("no image available");
	}
	return *image;
}

//////////////////////////////////////////////////////////////////////
// creating a dark image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Work to construct dark images
 */
ProcessingStep::state	DarkProcessor::do_work() {
	// common preparation work
	ProcessingStep::state	preparation = common_work();
	if (preparation != ProcessingStep::complete) {
		return preparation;
	}

	// create an adapter that is capable of computing the pixel values
	// for the dark

	// cheat
	return ProcessingStep::complete;
}

//////////////////////////////////////////////////////////////////////
// creating a flat image
//////////////////////////////////////////////////////////////////////
/**
 * \brief Work to construct flat images
 */
ProcessingStep::state	FlatProcessor::do_work() {
	// common preparation work
	ProcessingStep::state	preparation = common_work();
	if (preparation != ProcessingStep::complete) {
		return preparation;
	}

	// cheat
	return ProcessingStep::complete;
}

} // namespace process
} // namespace astro
