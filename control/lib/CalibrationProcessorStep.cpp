/*
 * CalibrationProcessorStep.cpp -- implementation of creators for calibration
 *                                 images
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <AstroDebug.h>

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
 * \brief Find all RawImageFile precursors
 */
size_t	CalibrationProcessor::getPrecursors() {
	typedef RawImageFile	*RawImageFilePtr;
	rawimages = new RawImageFilePtr[precursors().size()];
	nrawimages = 0;
/*
	std::for_each(precursors().begin(), precursors().end(),
		[rawimages,nrawimages](ProcessingStep *step) {
			RawImageFile	*f = dynamic_cast<RawImageFile *>(step);
			if (NULL != f) {
				rawimages[nrawimages++] = f;
			}
		}
	);
*/
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
	return ProcessingStep::complete;
}

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
