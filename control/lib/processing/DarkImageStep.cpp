/*
 * DarkImageStep.cpp -- DarkImageStep implementation
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <AstroCalibration.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new dark image
 */
DarkImageStep::DarkImageStep() {
	_badpixellimit = 3;
}

/**
 * \brief Perform the dark image computation
 */
ProcessingStep::state	DarkImageStep::do_work() {
	// construct the sequence of images
	ImageSequence	images;
	std::for_each(precursors().begin(), precursors().end(),
		[&images](int precursorid) mutable {
			ProcessingStepPtr p = ProcessingStep::byid(precursorid);
			if (!p) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d not remembered", precursorid);
			}
			ImageStep	*j = dynamic_cast<ImageStep*>(&*p);
			if (NULL == j) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d not an image step", j->id());
				return;
			}
			images.push_back(j->image());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "add image %d",
				j->id());
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d images", images.size());
	if (images.size() == 0) {
		return ProcessingStep::failed;
	}

	// actually produce the dark frame
	astro::calibration::DarkFrameFactory	dff(_badpixellimit);
	_image = dff(images);

	// remember that 
	time_t	now;
	time(&now);
	ProcessingStep::when(now);

	// return completion status
	return ProcessingStep::complete;
}

} // namespace process
} // namespace astro
