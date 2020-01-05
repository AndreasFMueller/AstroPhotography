/*
 * ImageStep.cpp -- processing steps that represent image
 *
 * This was historically the first piece of the project that used lambdas
 * in an essential way
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <AstroProcess.h>
#include <algorithm>
#include <includes.h>
#include <AstroDebug.h>
#include <algorithm>

using namespace astro::adapter;

namespace astro {
namespace process {

/**
 * \brief Get a vector of precursor images
 */
ImageSequence	ImageStep::precursorimages(std::vector<int> exclude) const {
	ImageSequence   images;
	std::for_each(precursors().begin(), precursors().end(),
		[&images,&exclude](int precursorid) mutable {
			ProcessingStepPtr p = ProcessingStep::byid(precursorid);
			if (!p) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d not remembered", precursorid);
			}
			ImageStep       *j = dynamic_cast<ImageStep*>(&*p);
			if (NULL == j) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"%d not an image step", j->id());
				return;
			}
			// if the image is not excluded add it
			if (exclude.end() != std::find(exclude.begin(),
				exclude.end(), j->id())) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"precursor %d excluded", j->id());
				return;
			}
			images.push_back(j->image());
			debug(LOG_DEBUG, DEBUG_LOG, 0, "add image %d",
				j->id());
		}
	);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d precursors", images.size());
	return images;
}

/**
 * \brief Retrieve the unique precursor image
 *
 * If there is not exactly one precursor image, throw an exception,
 * otherwise return the unique precursor image
 */
ImagePtr	ImageStep::precursorimage(std::vector<int> exclude) const {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting single precursor image");
	// get the image from the precursor
	ImageSequence	p = precursorimages(exclude);
	if (p.size() != 1) {
		std::string	msg = stringprintf("wrong number of precursor "
			"images: %d != 1", p.size());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	ImagePtr	precursorimage = *p.begin();
	return precursorimage;
}

class inconsistent_size : public std::exception {
};

/**
 * \brief Verify that the precursor images are consistent in size
 */
bool	ImageStep::precursorSizesConsistent(std::vector<int> exclude) const {
	ImageSequence	images = precursorimages(exclude);
	ImageSize	size = (*images.begin())->size();
	try {
		std::for_each(images.begin(), images.end(),
			[&](ImagePtr image) {
				if (image->size() != size) {
					throw inconsistent_size();
				}
			}
		);
	} catch (inconsistent_size) {
		return false;
	}
	return true;
}

} // namespace process
} // namespace astro
