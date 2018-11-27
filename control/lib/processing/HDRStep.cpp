/*
 * HDRStep.cpp -- implementation of the HDR step
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>

namespace astro {
namespace process {

/**
 * \brief Construct a new HDRStep
 */
HDRStep::HDRStep(NodePaths& parent) : ImageStep(parent) {
	_maskid = -1;
}

ProcessingStep::state	HDRStep::do_work() {
	try {
		// get the only remaining precursor, i.e. the image
		std::vector<int>	exclude;
		if (maskid() > 0) {
			exclude.push_back(maskid());
		}
		ImagePtr        precursor = precursorimage(exclude);

		// get the mask precursor
		if (maskid() > 0) {
			ProcessingStepPtr	maskstep = byid(maskid());
			ImageStep	*maskimagestep
				= dynamic_cast<ImageStep*>(&*maskstep);
			mask(maskimagestep->image());
		} else {
			Image<float>	*maskimg
				= new Image<float>(precursor->size());
			maskimg->fill(1);
			mask(ImagePtr(maskimg));
		}

		// apply HDR transformation
		_image = (*this)(precursor);
		return ProcessingStep::complete;
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "processing error: %s", x.what());
	}
	return ProcessingStep::failed;
}

std::string	HDRStep::what() const {
	return std::string("perform HDR transform");
}

} // namespace process
} // namespace astro
